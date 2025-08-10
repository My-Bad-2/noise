#ifndef KERNEL_VERSION_HPP
#define KERNEL_VERSION_HPP

#include <stddef.h>
#include <stdint.h>

#ifndef GIT_COMMIT_ID
#define GIT_COMMIT_ID "0"
#endif

struct KernelInfo {
 public:
  constexpr KernelInfo() : commit_id(GIT_COMMIT_ID), timestamp(__TIMESTAMP__), compiler_version(__VERSION__) {
  }

  void print() const;

 private:
  const char* commit_id;
  const char* timestamp;
  const char* compiler_version;
};

#endif  // KERNEL_VERSION_HPP
