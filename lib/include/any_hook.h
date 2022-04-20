#pragma once
#include <cstdint>
namespace AnyHook {
#pragma pack(push, 1)
struct RegisterData64 {
  uint64_t rax;
  uint64_t rbx;
  uint64_t rcx;
  uint64_t rdx;
  uint64_t rsi;
  uint64_t rdi;
  uint64_t rbp;
  uint64_t rsp;
  uint64_t r8;
  uint64_t r9;
  uint64_t r10;
  uint64_t r11;
  uint64_t r12;
  uint64_t r13;
  uint64_t r14;
  uint64_t r15;
  struct {
    uint64_t CF : 1;                // 0 Carry flag
    uint64_t reserved_1 : 1;        // 1 Reserved, always 1 in EFLAGS
    uint64_t PF : 1;                // 2 Parity flag
    uint64_t reserved_2 : 1;        // 3 Reserved[3]
    uint64_t AF : 1;                // 4 Adjust flag
    uint64_t reserved_3 : 1;        // 5 Reserved[3]
    uint64_t ZF : 1;                // 6 Zero flag
    uint64_t SF : 1;                // 7 Sign flag
    uint64_t TF : 1;                // 8 Trap flag (single step)
    uint64_t IF : 1;                // 9 Interrupt enable flag
    uint64_t DF : 1;                // 10 Direction flag
    uint64_t OF : 1;                // 11 Overflow flag
    uint64_t system_reserved : 52;  // 12-63

  } rflags;
  // xmm registers
};

struct RegisterData32 {
  uint32_t eax;
  uint32_t ebx;
  uint32_t ecx;
  uint32_t edx;
  uint32_t esi;
  uint32_t edi;
  uint32_t ebp;
  uint32_t esp;
  struct {
    uint32_t CF : 1;                // 0 Carry flag
    uint32_t reserved_1 : 1;        // 1 Reserved, always 1 in EFLAGS
    uint32_t PF : 1;                // 2 Parity flag
    uint32_t reserved_2 : 1;        // 3 Reserved[3]
    uint32_t AF : 1;                // 4 Adjust flag
    uint32_t reserved_3 : 1;        // 5 Reserved[3]
    uint32_t ZF : 1;                // 6 Zero flag
    uint32_t SF : 1;                // 7 Sign flag
    uint32_t TF : 1;                // 8 Trap flag (single step)
    uint32_t IF : 1;                // 9 Interrupt enable flag
    uint32_t DF : 1;                // 10 Direction flag
    uint32_t OF : 1;                // 11 Overflow flag
    uint32_t system_reserved : 20;  // 12-31

  } eflags;
  // xmm registers
};

#pragma pack(pop)
#ifdef _WIN64
using RegisterData = RegisterData64;
#else
using RegisterData = RegisterData32;
#endif

using FnHookCallbackType = void (*)(RegisterData*);

using Handle = uint64_t;
constexpr Handle Invalid = Handle(-1);

Handle install_hook(void* target, FnHookCallbackType callback);
bool uninstall_hook(Handle h);

}  // namespace AnyHook
