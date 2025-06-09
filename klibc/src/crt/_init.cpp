extern "C" void (*__init_array_start[])();
extern "C" void (*__init_array_end[])();

extern "C" void _init(void) {
  for (auto ctor = __init_array_start; ctor < __init_array_end; ctor++) {
    (*ctor)();
  }
}