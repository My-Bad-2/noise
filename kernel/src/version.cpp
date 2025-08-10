#include "log.hpp"
#include "version.hpp"

void KernelInfo::print() const {
  info("Noise ({})\n\tBuild Date: {}\n\tCompiled with: {}", this->commit_id,
       this->timestamp, this->compiler_version);
}
