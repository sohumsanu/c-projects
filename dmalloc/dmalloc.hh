#ifndef __DMALLOC_HH
#define __DMALLOC_HH 1
#include <cassert>
#include <cstdlib>
#include <cinttypes>
#include <cstdio>
#include <new>

/**
 * dmalloc(sz,file,line)
 *      malloc() wrapper. Dynamically allocate the requested amount `sz` of memory and
 *      return a pointer to it
 *
 * @arg size_t sz : the amount of memory requested
 * @arg const char *file : a string containing the filename from which dmalloc was called
 * @arg long line : the line number from which dmalloc was called
 *
 * @return a pointer to the heap where the memory was reserved
 */
void* dmalloc(size_t sz, const char* file, long line);

/**
 * dfree(ptr, file, line)
 *      free() wrapper. Release the block of heap memory pointed to by `ptr`. This should
 *      be a pointer that was previously allocated on the heap. If `ptr` is a nullptr do nothing.
 *
 * @arg void *ptr : a pointer to the heap
 * @arg const char *file : a string containing the filename from which dfree was called
 * @arg long line : the line number from which dfree was called
 */
void dfree(void* ptr, const char* file, long line);

/**
 * dcalloc(nmemb, sz, file, line)
 *      calloc() wrapper. Dynamically allocate enough memory to store an array of `nmemb`
 *      number of elements with wach element being `sz` bytes. The memory should be initialized
 *      to zero
 *
 * @arg size_t nmemb : the number of items that space is requested for
 * @arg size_t sz : the size in bytes of the items that space is requested for
 * @arg const char *file : a string containing the filename from which dcalloc was called
 * @arg long line : the line number from which dcalloc was called
 *
 * @return a pointer to the heap where the memory was reserved
 */
void* dcalloc(size_t nmemb, size_t sz, const char* file, long line);

// struct to store global information about the dalloc functions
struct dmalloc_stats {
    unsigned long long nactive;         // # active allocations
    unsigned long long active_size;     // # bytes in active allocations
    unsigned long long ntotal;          // # total allocations
    unsigned long long total_size;      // # bytes in total allocations
    unsigned long long nfail;           // # failed allocation attempts
    unsigned long long fail_size;       // # bytes in failed alloc attempts
    uintptr_t heap_min;                 // smallest allocated addr
    uintptr_t heap_max;                 // largest allocated addr
};

struct meta_data {
    size_t whole_size;
    size_t size;
    const char* file_name;
    long line_number;
    bool is_free;
    // char padding[100000];
    char* padding;
    char* padding1;
    char* padding2;
    char* padding3;
    char* padding4;
    char* padding5;
    char* padding6;
    char* padding7;
    char* padding8;
    char* padding9;
    char* padding10;
    char* padding11;
    char* padding12;
    char* padding13;
    char* padding14;
    char* padding15;
    char* padding16;
    char* padding17;
    char* padding18;
    char* padding19;
    char* padding20;
    char* padding21;
    char* padding22;
    char* padding23;
    char* padding24;
    char* padding25;
    char* padding26;
    char* padding27;
    char* padding28;
    char* padding29;
    char* padding30;
    char* padding31;
    char* padding32;
    char* padding33;
    char* padding34;
    char* padding35;
    char* padding36;
    char* padding37;
    char* padding38;
    char* padding39;
    char* padding40;
    char* padding41;
    char* padding42;
    char* padding43;
    char* padding44;
    char* padding45;
    char* padding46;
    char* padding47;
    char* padding48;
    char* padding49;
    char* padding112;
    char* padding212;
    char* padding312;
    char* padding421;
    char* padding51;
    char* padding612;
    char* padding712;
    char* padding821;
    char* padding9112;
    char* padding1112;
    char* padding1212;
    char* padding1123;
    char* padding1124;
    char* padding1125;
    char* padding1126;
    char* padding1127;
    char* padding1128;
    char* padding1129;
    char* padding2012;
    char* padding2121;
    char* padding2122;
    char* padding2213;
    char* padding2214;
    // char* padding2125;
    // char* padding2126;
    // char* padding2217;
    // char* padding2218;
    // char* padding2129;
    // char* padding3120;
    // char* padding3211;
    // char* padding3122;
    // char* padding3123;
    // char* padding3124;
    // char* padding3125;
    // char* padding3216;
    // char* padding3127;
    // char* padding3128;
    // char* padding3912;
    // char* padding4120;
    // char* padding4121;
    // char* padding4122;
    // char* padding4123;
    // char* padding4124;
    // char* padding4521;
    // char* padding4126;
    // char* padding4712;
    // char* padding4812;
    // char* padding4912;
    
    char secret[8];
};

extern struct dmalloc_stats statistics;

/**
 * get_statistics(stats)
 *      fill a dmalloc_stats pointer with the current memory statistics
 *
 * @arg dmalloc_stats *stats : a pointer to the the dmalloc_stats struct we want to fill
 */
void get_statistics(dmalloc_stats* stats);

/**
 * print_statistics()
 *      print the current memory statistics to stdout
 */
void print_statistics();

/**
 * print_leak_report()
 *      Print a report of all currently-active allocated blocks of dynamic
 *      memory.
 */
void print_leak_report();

// these functions model the base functionality for malloc free and clalloc
// `dmalloc.cc` should use these functions rather than malloc() and free().
void* base_malloc(size_t sz);
void base_free(void* ptr);
void base_allocator_disable(bool is_disabled);

/// Preprocessor macros to override system versions with our versions.
#if !DMALLOC_DISABLE
#define malloc(sz)          dmalloc((sz), __FILE__, __LINE__)
#define free(ptr)           dfree((ptr), __FILE__, __LINE__)
#define calloc(nmemb, sz)   dcalloc((nmemb), (sz), __FILE__, __LINE__)
#endif


/// This magic class lets standard C++ containers use your debugging allocator,
/// instead of the system allocator. Don't worry about this
template <typename T>
class dbg_allocator {
public:
    using value_type = T;
    dbg_allocator() noexcept = default;
    dbg_allocator(const dbg_allocator<T>&) noexcept = default;
    template <typename U> dbg_allocator(dbg_allocator<U>&) noexcept {}

    T* allocate(size_t n) {
        return reinterpret_cast<T*>(dmalloc(n * sizeof(T), "?", 0));
    }
    void deallocate(T* ptr, size_t) {
        dfree(ptr, "?", 0);
    }
};
template <typename T, typename U>
inline constexpr bool operator==(const dbg_allocator<T>&, const dbg_allocator<U>&) {
    return true;
}
template <typename T, typename U>
inline constexpr bool operator!=(const dbg_allocator<T>&, const dbg_allocator<U>&) {
    return false;
}

#endif
