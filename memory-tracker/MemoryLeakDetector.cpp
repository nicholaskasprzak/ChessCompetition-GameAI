#include "MemoryLeakDetector.h"
#include <mutex>

void *mynew(std::size_t size) {
  void *mem = std::malloc(size);
  auto *stats = memory_stats::get();
  stats->allocCount++;
  if (stats->allocCount > stats->maxAllocCount)
    stats->maxAllocCount = stats->allocCount;
  return mem;
}

void myDelete(void *mem) {
  auto *stats = memory_stats::get();
  stats->allocCount--;
  std::free(mem);
}

void *operator new(std::size_t size) { return mynew(size); }
void *operator new(std::size_t size, std::align_val_t alignment) {
  return mynew(size);
}
void *operator new(std::size_t size, const std::nothrow_t &) {
  return mynew(size);
}
void *operator new(std::size_t size, std::align_val_t alignment,
                   const std::nothrow_t &) {
  return mynew(size);
}
void operator delete(void *ptr) { myDelete(ptr); }
void operator delete(void *ptr, std::size_t size) { myDelete(ptr); }
void operator delete(void *ptr, std::align_val_t alignment) { myDelete(ptr); }
void operator delete(void *ptr, std::size_t size, std::align_val_t alignment) {
  myDelete(ptr);
}
void operator delete(void *ptr, const std::nothrow_t &) { myDelete(ptr); }
void operator delete(void *ptr, std::align_val_t alignment,
                     const std::nothrow_t &) {
  myDelete(ptr);
}
void *operator new[](std::size_t size) { return mynew(size); }
void *operator new[](std::size_t size, std::align_val_t alignment) {
  return mynew(size);
}
void *operator new[](std::size_t size, const std::nothrow_t &) {
  return mynew(size);
}
void *operator new[](std::size_t size, std::align_val_t alignment,
                     const std::nothrow_t &) {
  return mynew(size);
}
void operator delete[](void *ptr) { myDelete(ptr); }
void operator delete[](void *ptr, std::size_t size) { myDelete(ptr); }
void operator delete[](void *ptr, std::align_val_t alignment) { myDelete(ptr); }
void operator delete[](void *ptr, std::size_t size,
                       std::align_val_t alignment) {
  myDelete(ptr);
}
void operator delete[](void *ptr, const std::nothrow_t &) { myDelete(ptr); }
void operator delete[](void *ptr, std::align_val_t alignment,
                       const std::nothrow_t &) {
  myDelete(ptr);
}

// void* operator new  (std::size_t size, void* ptr) noexcept;
// void* operator new[](std::size_t size, void* ptr) noexcept;
// void  operator delete  (void* ptr, void*) noexcept;
// void  operator delete[](void* ptr, void*) noexcept;