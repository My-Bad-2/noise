#ifndef SPINLOCK_HPP
#define SPINLOCK_HPP 1

#include "arch/arch.hpp"

#include <stddef.h>
#include <stdint.h>

#include <atomic>

namespace libs {
enum class LockType {
  SpinlockSpin,
  SpinlockIrq,
};

template <LockType type>
class Spinlock {};

template <>
class Spinlock<LockType::SpinlockSpin> {
 public:
  constexpr Spinlock() : next_ticket(0), serving_ticket(0) {
  }

  Spinlock(const Spinlock&) = delete;
  Spinlock(Spinlock&&) = delete;

  Spinlock& operator=(const Spinlock&) = delete;
  Spinlock& operator=(Spinlock&&) = delete;

  void lock() {
    size_t ticket = this->next_ticket.fetch_add(1, std::memory_order_relaxed);

    while (this->serving_ticket.load(std::memory_order_acquire) != ticket) {
      arch::pause();
    }
  }

  bool unlock() {
    if (!is_locked()) {
      return false;
    }

    size_t curr = this->serving_ticket.load(std::memory_order_relaxed);
    this->serving_ticket.store(curr + 1, std::memory_order_release);

    return true;
  }

  bool is_locked() const {
    size_t curr = serving_ticket.load(std::memory_order_relaxed);
    size_t next = next_ticket.load(std::memory_order_relaxed);

    return curr != next;
  }

  bool try_lock() {
    if (is_locked()) {
      return false;
    }

    lock();

    return true;
  }

 private:
  std::atomic_size_t next_ticket;
  std::atomic_size_t serving_ticket;
};

template <>
class Spinlock<LockType::SpinlockIrq>
    : public Spinlock<LockType::SpinlockSpin> {
 public:
  constexpr Spinlock() : Spinlock<LockType::SpinlockSpin>(), interrupts(false) {
  }

  void lock() {
    Spinlock<LockType::SpinlockSpin>::lock();
    this->interrupts = arch::int_status();
    arch::int_switch(false);
  }

  bool unlock() {
    if (!Spinlock<LockType::SpinlockSpin>::unlock()) {
      return false;
    }

    if (arch::int_status() != interrupts) {
      arch::int_switch(interrupts);
    }

    return true;
  }

 private:
  bool interrupts;
};

struct DeferLock {
  explicit DeferLock() = default;
};

struct TryToLock {
  explicit TryToLock() = default;
};

struct AdoptLock {
  explicit AdoptLock() = default;
};

constexpr DeferLock defer_lock{};
constexpr TryToLock try_to_lock{};
constexpr AdoptLock adopt_lock{};

template <typename Mutex>
class LockGuard {
 public:
  using MutexType = Mutex;

  // --- Constructors ---

  // Default constructor: creates a lock that does not own a mutex.
  LockGuard() noexcept : m_mutex(nullptr), m_owns(false) {
  }

  // Acquires ownership and locks the given mutex.
  explicit LockGuard(MutexType& m) : m_mutex(&m), m_owns(false) {
    m_mutex->lock();
    m_owns = true;
  }

  // Does not lock the mutex on construction.
  LockGuard(MutexType& m, DeferLock) noexcept : m_mutex(&m), m_owns(false) {
  }

  // Tries to lock the mutex on construction without blocking.
  LockGuard(MutexType& m, TryToLock) : m_mutex(&m), m_owns(m.try_lock()) {
  }

  // Assumes the calling thread already owns the mutex.
  LockGuard(MutexType& m, AdoptLock) noexcept : m_mutex(&m), m_owns(true) {
  }

  // Move constructor: transfers ownership from another UniqueLock.
  LockGuard(LockGuard&& other) noexcept
      : m_mutex(other.m_mutex), m_owns(other.m_owns) {
    other.m_mutex = nullptr;
    other.m_owns = false;
  }

  // --- Destructor ---
  ~LockGuard() {
    if (m_owns) {
      m_mutex->unlock();
    }
  }

  LockGuard(const LockGuard&) = delete;
  LockGuard& operator=(const LockGuard&) = delete;

  LockGuard& operator=(LockGuard&& other) noexcept {
    if (this != &other) {
      // If we currently own a lock, release it.
      if (m_owns) {
        m_mutex->unlock();
      }

      // Steal resources from the other lock.
      m_mutex = other.m_mutex;
      m_owns = other.m_owns;

      // Leave the other lock in a safe, empty state.
      other.m_mutex = nullptr;
      other.m_owns = false;
    }

    return *this;
  }

  // --- Locking Operations ---

  void lock() {
    if (!m_mutex) {
      return;
    }

    if (m_owns) {
      return;
    }

    m_mutex->lock();
    m_owns = true;
  }

  bool try_lock() {
    if (!m_mutex) {
      return false;
    }

    if (m_owns) {
      return false;
    }

    m_owns = m_mutex->try_lock();
    return m_owns;
  }

  void unlock() {
    if (!m_owns) {
      return;
    }

    m_mutex->unlock();
    m_owns = false;
  }

  MutexType* release() noexcept {
    MutexType* released_mutex = m_mutex;
    m_mutex = nullptr;
    m_owns = false;
    return released_mutex;
  }

  void swap(LockGuard& other) noexcept {
    std::swap(m_mutex, other.m_mutex);
    std::swap(m_owns, other.m_owns);
  }

  bool owns_lock() const noexcept {
    return m_owns;
  }

  explicit operator bool() const noexcept {
    return m_owns;
  }

  MutexType* mutex() const noexcept {
    return m_mutex;
  }

 private:
  MutexType* m_mutex;
  bool m_owns;
};

template<class Mutex>
LockGuard(Mutex&) -> LockGuard<Mutex>;

using SpinLock = Spinlock<LockType::SpinlockSpin>;
using IrqLock = Spinlock<LockType::SpinlockIrq>;
}  // namespace libs

#endif  // SPINLOCK_HPP
