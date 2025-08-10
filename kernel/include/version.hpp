#ifndef KERNEL_VERSION_HPP
#define KERNEL_VERSION_HPP

#include <stddef.h>
#include <stdint.h>

#ifndef GIT_COMMIT_ID
#define GIT_COMMIT_ID "0"
#endif

#ifndef KERNEL_VERSION
#define KERNEL_VERSION "0.0.1"
#endif

#ifndef KERNEL_NAME
#define KERNEL_NAME "Noise"
#endif

struct KernelInfo {
 public:
  constexpr KernelInfo()
      : name(KERNEL_NAME),
        version(KERNEL_VERSION),
        commit_id(GIT_COMMIT_ID),
        timestamp(__TIMESTAMP__),
        compiler_version(__VERSION__) {
  }

  void print() const;

 private:
  const char* name;
  const char* version;
  const char* commit_id;
  const char* timestamp;
  const char* compiler_version;
};

#endif  // KERNEL_VERSION_HPP
