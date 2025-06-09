extern "C" void (*__fini_array_start[])();
extern "C" void (*__fini_array_end[])();

extern "C" void _fini() {
  for (auto dtor = __fini_array_start; dtor < __fini_array_end; dtor++) {
    (*dtor)();
  }
}