#ifndef MemoryLeakTracker_h
#define MemoryLeakTracker_h

#include <exception>
#include <iostream>
#include <map>
#include <memory>
#include <new>

void *operator new(std::size_t size); // replaceable, nodiscard in C++20
void *operator new(
    std::size_t size,
    std::align_val_t alignment); // replaceable, C++17, nodiscard in C++20
void *operator new(std::size_t size,
                   const std::nothrow_t &); // replaceable, nodiscard in C++20
void *
operator new(std::size_t size, std::align_val_t alignment,
             const std::nothrow_t &); // replaceable, C++17, nodiscard in C++20
void operator delete(void *ptr);      // replaceable
void operator delete(void *ptr, std::size_t size); // replaceable, C++14
void operator delete(void *ptr,
                     std::align_val_t alignment); // replaceable, C++17
void operator delete(void *ptr, std::size_t size,
                     std::align_val_t alignment);        // replaceable, C++17
void operator delete(void *ptr, const std::nothrow_t &); // replaceable
void operator delete(void *ptr, std::align_val_t alignment,
                     const std::nothrow_t &); // replaceable, C++17

void *operator new[](std::size_t size); // replaceable, nodiscard in C++20
void *operator new[](
    std::size_t size,
    std::align_val_t alignment); // replaceable, C++17, nodiscard in C++20
void *operator new[](std::size_t size,
                     const std::nothrow_t &); // replaceable, nodiscard in C++20
void *operator new[](
    std::size_t size, std::align_val_t alignment,
    const std::nothrow_t &);       // replaceable, C++17, nodiscard in C++20
void operator delete[](void *ptr); // replaceable
void operator delete[](void *ptr, std::size_t size); // replaceable, C++14
void operator delete[](void *ptr,
                       std::align_val_t alignment); // replaceable, C++17
void operator delete[](void *ptr, std::size_t size,
                       std::align_val_t alignment);        // replaceable, C++17
void operator delete[](void *ptr, const std::nothrow_t &); // replaceable
void operator delete[](void *ptr, std::align_val_t alignment,
                       const std::nothrow_t &); // replaceable, C++17

// void *operator new(std::size_t size, void *ptr) noexcept; // nodiscard in
// C++20 void *operator new[](std::size_t size, void *ptr) noexcept; //
// nodiscard in C++20 void operator delete(void *ptr, void *) noexcept; void
// operator delete[](void *ptr, void *) noexcept;

struct memory_stats {
  int64_t allocCount = 0;
  int64_t maxAllocCount = 0;

private:
  memory_stats() = default;

public:
  memory_stats(const memory_stats &) = delete;
  memory_stats(memory_stats &&) = delete;
  static memory_stats *get() {
    static memory_stats stats;
    return &stats;
  }
  static const int64_t &count() { return get()->allocCount; }
  ~memory_stats() {
    std::cout << "Max alloc count: " << maxAllocCount << std::endl;
    if (allocCount != 0) {
      std::cerr << "Memory leak detected: " << allocCount
                << " allocations not deleted system wide" << std::endl;
    }
  }
};

#endif