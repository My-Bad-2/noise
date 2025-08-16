#ifndef LAZY_HPP
#define LAZY_HPP 1

#include <stddef.h>

#include <new>
#include <utility>

namespace libs {

namespace details {
template <size_t Size, size_t Align>
struct alignas(Align) Storage {
  constexpr Storage() : buffer{0} {
  }

  char buffer[Size];
};
}  // namespace details

template <typename T>
class Lazy {
 public:
  constexpr Lazy() : initialized(false) {
  }

  template <typename... Args>
  void initialize(Args &&...args) {
    if (this->initialized) {
      std::unreachable();
    }

    new (&this->storage.buffer) T(std::forward<Args>(args)...);
    this->initialized = true;
  }

  template <typename F, typename... Args>
  void construct_with(F f) {
    if (this->initialized) {
      std::unreachable();
    }

    new (&this->storage.buffer) T{f()};
    this->initialized = true;
  }

  void destruct() {
    if (!this->initialized) {
      std::unreachable();
    }

    get()->T::~T();
    this->initialized = false;
  }

  T *get() {
    if (!this->initialized) {
      std::unreachable();
    }

    return std::launder(reinterpret_cast<T *>(&this->storage.buffer));
  }

  bool valid() {
    return this->initialized;
  }

  explicit operator bool() {
    return this->initialized;
  }

  T *operator->() {
    return get();
  }

  T &operator*() {
    return *get();
  }

 private:
  details::Storage<sizeof(T), alignof(T)> storage;
  bool initialized;
};
}  // namespace libs

#endif  // LAZY_HPP
