#ifndef MemoryLeakTracker_h
#define MemoryLeakTracker_h

#include <exception>
#include <iostream>
#include <map>
#include <memory>
#include <new>

void* operator new(std::size_t size);                                   // replaceable, nodiscard in C++20
void* operator new(std::size_t size, std::align_val_t alignment);       // replaceable, C++17, nodiscard in C++20
void* operator new(std::size_t size, const std::nothrow_t&);   // replaceable, nodiscard in C++20
void* operator new(std::size_t size, std::align_val_t alignment, const std::nothrow_t&);                     // replaceable, C++17, nodiscard in C++20
void  operator delete(void* ptr);                              // replaceable
void  operator delete(void* ptr, std::size_t size);            // replaceable, C++14
void  operator delete(void* ptr, std::align_val_t alignment);  // replaceable, C++17
void  operator delete(void* ptr, std::size_t size, std::align_val_t alignment);             // replaceable, C++17
void  operator delete(void* ptr, const std::nothrow_t&);       // replaceable
void  operator delete(void* ptr, std::align_val_t alignment, const std::nothrow_t&);                  // replaceable, C++17

void* operator new[](std::size_t size);                                 // replaceable, nodiscard in C++20
void* operator new[](std::size_t size, std::align_val_t alignment);              // replaceable, C++17, nodiscard in C++20
void* operator new[](std::size_t size, const std::nothrow_t&); // replaceable, nodiscard in C++20
void* operator new[](std::size_t size, std::align_val_t alignment, const std::nothrow_t&);                   // replaceable, C++17, nodiscard in C++20
void  operator delete[](void* ptr);                            // replaceable
void  operator delete[](void* ptr, std::size_t size);          // replaceable, C++14
void  operator delete[](void* ptr, std::align_val_t alignment);           // replaceable, C++17
void  operator delete[](void* ptr, std::size_t size, std::align_val_t alignment);           // replaceable, C++17
void  operator delete[](void* ptr, const std::nothrow_t&);     // replaceable
void  operator delete[](void* ptr, std::align_val_t alignment, const std::nothrow_t&);                // replaceable, C++17

void* operator new  (std::size_t size, void* ptr) noexcept;             // nodiscard in C++20
void* operator new[](std::size_t size, void* ptr) noexcept;             // nodiscard in C++20
void  operator delete  (void* ptr, void*) noexcept;
void  operator delete[](void* ptr, void*) noexcept;

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

    void deallocate(T* p, size_t) {
        std::free(p);
    }
};

template <typename Key, typename Value, typename Comparator>
class ThreadSafeMap {
public:
    void insert(const Key& key, const Value& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        data_[key] = value;
    }

    bool get(const Key& key, Value& value) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = data_.find(key);
        if (it != data_.end()) {
            value = it->second;
            return true;
        }
        return false;
    }

    void remove(const Key& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        data_.erase(key);
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return data_.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return data_.size();
    }

    std::vector<std::pair<Key, Value>> pairs() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::pair<Key, Value>> result;
        for (auto& it : data_)
            result.push_back(it);
        return result;
    }

    // insert or update
    void upsert(const Key& key, const Value& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        data_[key] = value;
    }

    // contains
    bool contains(const Key& key) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return data_.find(key) != data_.end();
    }

    // erase
    void erase(const Key& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        data_.erase(key);
    }

    // at (read only)
    const Value& at(const Key& key) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return data_.at(key);
    }

private:
    std::map<Key, Value, Comparator, track_alloc<std::pair<const Key, Value>>> data_;
    mutable std::mutex mutex_;
};

typedef ThreadSafeMap<void*, std::size_t, std::less<void*>> track_type;


struct track_printer {
private:
    track_printer() = default;
public:
    track_type map;
    static track_printer* get() {
        static track_printer printer;
        return &printer;
    }
    ~track_printer() {
        uint64_t total = 0;
        for (auto & pair : map.pairs()) {
            std::cerr << "TRACK: leaked at " << pair.first << ", " << pair.second << " bytes\n";
            total += pair.second;
        }
        if(!map.empty())
            std::cerr << "TRACK: " << map.size() << " allocations leaked, total: " << total << " bytes\n";
    }
};

struct memory_stats {
    bool enabled=false;
    int64_t allocCount=0;
    int64_t maxAllocCount=0;
    int64_t allocSize=0;
    int64_t maxAllocSize=0;
private:
    memory_stats() = default;
public:
    memory_stats(const memory_stats&) = delete;
    memory_stats(memory_stats&&) = delete;
    static memory_stats* get() {
        static memory_stats stats;
        return &stats;
    }
    ~memory_stats() {
        std::cout << "Peak memory usage: " << maxAllocSize << " bytes" << std::endl;
        std::cout << "Max alloc count: " << maxAllocCount << std::endl;
      if(allocCount!=0) {
          std::cerr << "Memory leak detected: " << allocCount << " allocations not freed. Possibly from the libraries"
                    << std::endl;
          std::cerr << "Total memory leaked: " << allocSize << " bytes" << std::endl;
      }
    }
};

#endif