#include "any_hook_impl.h"
#include "../../third-party/minhook/include/MinHook.h"

extern "C" {
// target->trampoline->stub->prox->callback->origional
#ifdef _WIN64
uint64_t __fastcall any_hook_proxy64(AnyHook::AnyHookItem* hook, AnyHook::RegisterData64* register_data) {
  //恢复到最原始栈地址
  register_data->rsp += 0x110;
  hook->fn_callback(register_data);
  register_data->rsp -= 0x110;
  return (uint64_t)hook->fn_original;
}
void any_hook_stub64();

#else
uint32_t __fastcall any_hook_proxy32(AnyHook::AnyHookItem* hook, AnyHook::RegisterData32* register_data) {
  //恢复到最原始栈地址
  register_data->esp += 0x108;
  hook->fn_callback(register_data);
  register_data->esp -= 0x108;
  return (uint32_t)hook->fn_original;
}
_declspec(naked) void any_hook_stub32() {
  __asm {
        mov [esp-100h],ecx
pop ecx
sub esp, 100h;
mov [esp],	  eax
mov [esp+4h], ebx
mov [esp+8h], ecx
mov [esp+0Ch],edx
mov [esp+10h],esi
mov [esp+14h],edi
mov [esp+18h],ebp
mov [esp+01Ch],esp
;get rflags
mov eax, [esp+100h];
mov [esp+20h],eax

;get context pointer
mov ecx, [esp-4h];

mov edx, esp
;rbp save by callee
mov ebp, esp
;16 bit align
and esp, -10h

call any_hook_proxy32
;restore rsp
mov esp, ebp

;ret to origion
mov [esp+104h],eax
;get changed rflags
mov eax, [esp+20h]
mov [esp+100h],eax

mov eax, [esp]  
mov ebx, [esp+4h]
mov ecx, [esp+8h]
mov edx, [esp+0Ch]
mov esi, [esp+10h]
mov edi, [esp+14h]
mov ebp, [esp+18h]
mov esp, [esp+01Ch]
add esp, 100h
;sub or add changed flags, so restore flags last
popfd
ret
  }
}

#endif
}
namespace AnyHook {

static Handle inline make_handle(uint32_t magic, uint32_t index) {
  return (((uint64_t)magic << 32) & 0xFFFFFFFF00000000) | index;
}
static uint32_t inline get_magic(Handle h) {
  return (h >> 32) & 0xFFFFFFFF;
}
static uint32_t inline get_index(Handle h) {
  return h & 0xFFFFFFFF;
}

AnyHookItem::AnyHookItem(void* target, FnHookCallbackType callback, void* trampoline, uint32_t magic)
    : fn_target(target), fn_callback(callback), trampoline_instr(trampoline), magic(magic) {
  HookTrampolineInstruction ins;
#ifdef _WIN64
  ins.rcx_imm = (uint64_t)this;
  ins.jmp_imm = (uint64_t)&any_hook_stub64;
#else
  ins.ecx_imm = (uint32_t)this;
  ins.jmp_imm = (uint32_t)&any_hook_stub32 - ((uint32_t)trampoline + sizeof(ins));
#endif
  memcpy(trampoline_instr, &ins, sizeof(ins));

  MH_STATUS status = MH_CreateHook(fn_target, trampoline_instr, &fn_original);
  status = MH_EnableHook(fn_target);
}

AnyHookItem::~AnyHookItem() {
  MH_RemoveHook(fn_target);
}

AnyHook::AnyHookMgr& AnyHookMgr::instance() {
  static AnyHookMgr mgr;
  return mgr;
}

AnyHookMgr::~AnyHookMgr() {}

AnyHook::Handle AnyHookMgr::install_hook(void* target, FnHookCallbackType callback) {
  std::lock_guard<std::mutex> l(lock);
  if (free_size == 0) {
    allocate_hook_item();
  }

  if (next_free >= hooks.size()) {
    return Invalid;
  }

  int cur_free = next_free;
  const int magic = GetTickCount() + magic_num++;
  const int trampoline_idx = cur_free / s_page_trampoline_size;
  if (trampoline_idx < trampoline_intrs.size() && trampoline_intrs[trampoline_idx]) {
    hooks[cur_free].item =
        std::make_unique<AnyHookItem>(target,
                                      callback,
                                      (uint8_t*)trampoline_intrs[trampoline_idx] +
                                          (sizeof(HookTrampolineInstruction) * (cur_free % s_page_trampoline_size)),
                                      magic);
    next_free = hooks[cur_free].next_free;
    --free_size;
    return make_handle(magic, cur_free);
  }
  return Invalid;
}

bool AnyHookMgr::uninstall_hook(Handle h) {
  std::lock_guard<std::mutex> l(lock);
  const size_t idx = get_index(h);
  const int magic = get_magic(h);
  if (idx < hooks.size() && hooks[idx].item && hooks[idx].item->magic == magic) {
    hooks[idx].item = nullptr;
    hooks[idx].next_free = next_free;
    next_free = idx;
    ++free_size;
    return true;
  }
  return false;
}

AnyHookMgr::AnyHookMgr() {
  MH_Initialize();
}

void AnyHookMgr::allocate_hook_item() {
  if (free_size == 0) {
    size_t pre_size = hooks.size();
    hooks.resize(hooks.size() + s_page_trampoline_size);
    for (int i = 0; i < s_page_trampoline_size; ++i) {
      hooks[pre_size + i].next_free = pre_size + i + 1;
    }
    free_size = s_page_trampoline_size;
    next_free = pre_size;

    void* ptr = VirtualAlloc(nullptr, s_page_align, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    trampoline_intrs.emplace_back(ptr);
  }
}

Handle install_hook(void* target, FnHookCallbackType callback) {
  return AnyHook::AnyHookMgr::instance().install_hook(target, callback);
}
bool uninstall_hook(Handle h) {
  if (h == Invalid) {
    return false;
  }
  return AnyHook::AnyHookMgr::instance().uninstall_hook(h);
}

}  // namespace AnyHook