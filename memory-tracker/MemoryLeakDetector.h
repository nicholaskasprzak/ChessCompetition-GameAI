#ifndef MemoryLeakTracker_h
#define MemoryLeakTracker_h

#include <exception>
#include <iostream>
#include <map>
#include <memory>

template <typename T> struct track_alloc : std::allocator<T> {
  template <typename U> struct rebind { typedef track_alloc<U> other; };

  track_alloc() {}

  template <typename U> track_alloc(track_alloc<U> const& u) : std::allocator<T>(u) {}

  T* allocate(size_t size, const T* = 0) {
    void* p = std::malloc(size * sizeof(T));
    if (p == nullptr) {
      throw std::bad_alloc();
    }
    return static_cast<T*>(p);
  }

  void deallocate(T* p, size_t) { std::free(p); }
};

typedef std::map<void*, std::size_t, std::less<void*>,
                 track_alloc<std::pair<void* const, std::size_t> > >
    track_type;

struct track_printer {
  track_type* track;
  track_printer(track_type* track) : track(track) {}
  ~track_printer() {
    track_type::const_iterator it = track->begin();
    while (it != track->end()) {
      std::cerr << "TRACK: leaked at " << it->first << ", " << it->second << " bytes\n";
      ++it;
    }
  }
};

track_type* get_map();

#endif