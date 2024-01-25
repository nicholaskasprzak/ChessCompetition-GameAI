#include "MemoryLeakDetector.h"

track_type* get_map() {
  // don't use normal new to avoid infinite recursion.
  static track_type* track = new (std::malloc(sizeof *track)) track_type;
  static track_printer printer(track);
  return track;
}

void* operator new(std::size_t size) noexcept(false) {
  // we are required to return non-null
  void* mem = std::malloc(size == 0 ? 1 : size);
  if (mem == 0) {
    throw std::bad_alloc();
  }
  (*get_map())[mem] = size;
  return mem;
}

void operator delete(void* mem) noexcept {
  if (get_map()->erase(mem) == 0) {
    // this indicates a serious bug
    std::cerr << "bug: memory at " << mem << " wasn't allocated by us\n";
  }
  std::free(mem);
}