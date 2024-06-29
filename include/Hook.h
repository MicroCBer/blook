#pragma once

#include "utils.h"
namespace blook {
class InlineHook {
  void *target;
  void *hook_func;
  size_t trampoline_size = 0;
  bool installed = false;

protected:
  void *p_trampoline = nullptr;

public:
  CLASS_MOVE_ONLY(InlineHook)
  InlineHook(void *target, void *hook_func);
  void *trampoline_raw();
  void install(bool try_trampoline = true);
};

template <typename ReturnVal, typename... Args>
struct InlineHookT : public InlineHook {
  inline constexpr InlineHookT(void *target, void *hook_func)
      : InlineHook(target, hook_func) {}
  inline auto trampoline() -> ReturnVal (*)(Args...) {
    return reinterpret_cast<ReturnVal (*)(Args...)>(p_trampoline);
  }
};
template <typename... T> using SIH = std::shared_ptr<InlineHookT<T...>>;
} // namespace blook