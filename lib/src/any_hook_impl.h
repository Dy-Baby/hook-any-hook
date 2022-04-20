#pragma once
#include <memory>
#include <mutex>
#include <vector>
#include "../include/any_hook.h"
namespace AnyHook {

#pragma pack(push, 1)

struct HookTrampolineInstruction32 {
  uint8_t push_ret = 0x51;  // use for store original address
  uint8_t pushfd = 0x9C;    // push eflags to stack
  uint8_t push_ecx = 0x51;

  uint8_t mov_rcx = 0xB9;  // move this ptr to rcx
  uint32_t ecx_imm;
  uint8_t jmp = 0xE9;
  uint32_t jmp_imm;
};

struct HookTrampolineInstruction64 {
  uint8_t push_ret = 0x51;  // use for store original address, at the end jmp to this address
  uint8_t pushfq = 0x9C;    // push rflags to stack
  uint8_t push_rcx = 0x51;

  uint16_t mov_rcx = 0xB948;  // move this ptr to rcx
  uint64_t rcx_imm;
  uint16_t jmp = 0x25FF;  // jmp to proxy
  uint32_t jmp_dummy = 0;
  uint64_t jmp_imm;
};
#pragma pack(pop)

#ifdef _WIN64
using HookTrampolineInstruction = HookTrampolineInstruction64;

#else
using HookTrampolineInstruction = HookTrampolineInstruction32;
#endif
static constexpr int s_page_align = 0x1000;
// size of trampolines on each memory page
static constexpr int s_page_trampoline_size = s_page_align / sizeof(HookTrampolineInstruction);

struct AnyHookItem {
  AnyHookItem(void* target, FnHookCallbackType callback, void* trampoline, uint32_t magic);
  ~AnyHookItem();
  uint32_t magic;
  void* trampoline_instr;
  void* fn_target;
  void* fn_original;
  FnHookCallbackType fn_callback;
};

struct AnyHookItemLink {
  std::unique_ptr<AnyHookItem> item;
  std::size_t next_free;
};
class AnyHookMgr {
 public:
  static AnyHookMgr& instance();
  ~AnyHookMgr();
  Handle install_hook(void* target, FnHookCallbackType callback);
  bool uninstall_hook(Handle h);

 private:
  AnyHookMgr();
  AnyHookMgr(AnyHookMgr&) = delete;

  void allocate_hook_item();

 private:
  int magic_num = 0x12debe;
  int free_size = 0;
  std::size_t next_free = -1;
  std::vector<AnyHookItemLink> hooks;
  std::vector<void*> trampoline_intrs;
  std::mutex lock;
};
}  // namespace AnyHook