#ifndef KERNEL_DRIVERS_MANAGER_HPP
#define KERNEL_DRIVERS_MANAGER_HPP

#include <stddef.h>
#include <stdint.h>

namespace drivers {
constexpr size_t INVALID_DRIVER_ID = static_cast<size_t>(-1);

class IDriver {
 public:
  IDriver() : m_driver_id(INVALID_DRIVER_ID) {};
  virtual bool initialize() = 0;
  virtual void shutdown() = 0;

  size_t get_driver_id() const {
    return this->m_driver_id;
  }

  void set_driver_id(const size_t id) {
    this->m_driver_id = id;
  }

 private:
  size_t m_driver_id;
};

size_t install(IDriver* driver);
void uninstall(IDriver* driver);

void initialize();
}  // namespace drivers

#endif  // KERNEL_DRIVERS_MANAGER_HPP
