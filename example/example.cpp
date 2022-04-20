#include <Windows.h>
#include <iostream>
#include "../lib/include/any_hook.h"

#ifdef _WIN64
void msg_box_hook(AnyHook::RegisterData* data) {
  data->rdx = (uint64_t)L"abc-changed";
}
#else
void msg_box_hook(AnyHook::RegisterData* data) {
  *((uint32_t*)data->esp + 2) = (uint32_t)L"abc-changed";
  data->eflags.TF = 1;
}
#endif
int main() {
  HMODULE user32 = LoadLibrary(L"user32.dll");
  for (int i = 0; i < 500; ++i) {
    AnyHook::Handle h = AnyHook::install_hook((uint8_t*)GetProcAddress(user32, "MessageBoxW"), msg_box_hook);
    if (i % 10 == 0) {
      AnyHook::uninstall_hook(h);
    }
  }

  MessageBoxW(nullptr, L"abc", L"abc", MB_OK);
  // AnyHook::uninstall_hook(h);
  AnyHook::Handle h1 = AnyHook::install_hook((uint8_t*)GetProcAddress(user32, "MessageBoxExW"), msg_box_hook);

  MessageBoxW(nullptr, L"abc", L"abc", MB_OK);
  MessageBoxExW(nullptr, L"", L"", MB_OK, 0);
  AnyHook::uninstall_hook(h1);
  MessageBoxExW(nullptr, L"", L"", MB_OK, 0);
  return 0;
}