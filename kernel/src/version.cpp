#include "log.hpp"
#include "version.hpp"

void KernelInfo::print() const {
  info("\n\t%s %s (commit: %s)\n\tBuild Date: %s\n\tCompiled with: %s", this->name,
       this->version, this->commit_id, this->timestamp, this->compiler_version);
}
