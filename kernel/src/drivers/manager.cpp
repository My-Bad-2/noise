#include "drivers/manager.hpp"

// static for now
#define MAX_DRIVERS 10

namespace drivers {
namespace {
IDriver *drivers[MAX_DRIVERS] = {nullptr};
size_t total_drivers = 0;

// TODO: Generate random driver id in future.
inline size_t allocate_driver_id() {
  return total_drivers++;
}
}  // namespace

size_t install(IDriver *driver) {
  const size_t device_id = allocate_driver_id();
  driver->set_driver_id(device_id);
  drivers[device_id] = driver;

  return device_id;
}

void uninstall(IDriver *driver) {
  // TODO: Match the driver signature and remove from the hashmap.
  // TODO: Implement after virtual allocation is supported.
}

void initialize() {
  for (size_t i = 0; i < total_drivers; i++) {
    if (drivers[i] == nullptr) {
      continue;
    }

    if (!drivers[i]->initialize()) {
      // TODO: Log error message and maybe, try again
      drivers[i]->set_driver_id(INVALID_DRIVER_ID);
      uninstall(drivers[i]);
    }
  }
}
}  // namespace drivers