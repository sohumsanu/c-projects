#define DMALLOC_DISABLE 1
#include "dmalloc.hh"
#include <cassert>
#include <cstring>
#include <unordered_map>
#include <string.h>


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

struct dmalloc_stats statistics = {0, 0, 0, 0, 0, 0, 0, 0};

bool has_errored = false;

std::unordered_map<long, meta_data> pointer_vals;

void* dmalloc(size_t sz, const char* file, long line) {
    (void) file, (void) line;   // avoid uninitialized variable warnings
    // Your code here.
    int padding = 0;
    meta_data* pointer = nullptr;
   if((sizeof(meta_data)+sz) > 0xFFFFFFFF || sz > 0xFFFFFFFF){
        statistics.nfail++;
        statistics.fail_size += sz;
        return nullptr;
    }

    // if((sizeof(meta_data) + sz % 16) == 0) {
    //     pointer = (meta_data*) base_malloc(sizeof(meta_data) + sz); // Do I have to add memory padding to this?
    // } else {
    //     for(int i =0; i<16; i++){
    //         if (sizeof(meta_data)+sz+i %16 == 0){
    //             pointer = (meta_data*) base_malloc(sizeof(meta_data) + i + sz); // Do I have to add memory padding to this?
    //             break;
    //         }
    //     }
    // }
    pointer = (meta_data*) base_malloc(sizeof(meta_data) + padding + sz + 8);
    // The padding will be different for an x32 or x64 bit system because of size_t -> so then what can I do (???)
    if (pointer == nullptr){
        statistics.nfail++;
        statistics.fail_size += sz;
        return pointer;
    }
    pointer->whole_size = sizeof(meta_data)+ padding +sz+8;
    pointer->size = sz;
    pointer->file_name = file;
    pointer->line_number = line;
    pointer->is_free = false;
    char* cpystr = pointer->secret;
    char str[] = "$Hu&n0Q";
    for(int i =0; i < 7; i++){
        cpystr[i] = str[i];
    }
    cpystr[7] = '\0';
    // The lines above set the meta_data struct to the correct values.
    
    void* ptr = (void*) (pointer+1);
    char* char_ptr = (char*) (pointer +1);
    char_ptr = (char_ptr + (padding+sz));
    strcpy(char_ptr, (pointer->secret));
    // The next lines skip the meta_data struct and return the final pointer.
    
    if(statistics.ntotal == 0){
        statistics.heap_min = (uintptr_t) ptr;
        statistics.heap_max = (uintptr_t) ptr + (uintptr_t)(pointer->whole_size);
    } else {
        // meta_data* min = (meta_data*) statistics.heap_min;
        // meta_data* max = (meta_data*) statistics.heap_max;
        // min--;
        // max--;
        // size_t amin = min->size;
        // size_t amax = max->size;
        if((uintptr_t) ptr < statistics.heap_min){ // (((meta_data*) statistics->heap_min)-1)->size - would this work inside the "if" statement?
            statistics.heap_min = (uintptr_t) ptr;
        } else if( (uintptr_t) ptr > statistics.heap_max){
            statistics.heap_max = (uintptr_t) ptr + pointer->whole_size;
        }
    }
    //The lines above set the heap_min and heap_max values.
    statistics.nactive++;
    statistics.ntotal++;
    statistics.total_size += sz;
    statistics.active_size += sz;
    if((pointer_vals.count((long) ptr))){
        meta_data* meta = &pointer_vals[(long) ptr];
        meta->is_free = false;
    } else {
        meta_data meta;
        meta.is_free = false;
        pointer_vals.insert({(long) ptr, meta});
    }
    //The lines above update the statistics struct.
    return ptr;
}

/**
 * dfree(ptr, file, line)
 *      free() wrapper. Release the block of heap memory pointed to by `ptr`. This should 
 *      be a pointer that was previously allocated on the heap. If `ptr` is a nullptr do nothing. 
 * 
 * @arg void *ptr : a pointer to the heap 
 * @arg const char *file : a string containing the filename from which dfree was called 
 * @arg long line : the line number from which dfree was called 
 */
void dfree(void* ptr, const char* file, long line) {
    (void) file, (void) line;   // avoid uninitialized variable warnings
    // Your code here.
    if(ptr == nullptr){
        return;
    } else if (has_errored){
        return;
    } else if (!((uintptr_t) ptr >= statistics.heap_min && (uintptr_t) ptr < statistics.heap_max)) {
        fprintf(stderr, "MEMORY BUG: invalid free of pointer %p, not in heap", ptr);
        has_errored = true;
        return;
    } else if (!(pointer_vals.count((long) ptr))) {
        fprintf(stderr, "MEMORY BUG: %s:%li: invalid free of pointer %p, not allocated\n", file, line, ptr);
        for (auto i : pointer_vals) {
            meta_data* pointer = (meta_data*) i.first;
            pointer--;
            if((long) ptr >= i.first && (long)ptr <= (long) (i.first+ (long) pointer->size)){
                fprintf(stderr, "%s:%li: %p is %li bytes inside a %li byte region allocated here", pointer->file_name, pointer->line_number, ptr, ((long)ptr -i.first), pointer->size);
                 has_errored = true;
                 return;
            }
        }
       
     }
    
    meta_data* pointer = (meta_data*) ptr;
    pointer--;
    // printf("pointer_vals[ptr]: %d", pointer_vals[(long) ptr]);
    // meta_data* meta = &pointer_vals[(long) ptr];
    if(pointer->is_free == true){
        fprintf(stderr, "MEMORY BUG: %s:%li: invalid free of pointer %p, double free", file, line, ptr);
        has_errored = true;
        return;
    }
    char* mvstr = (char*)pointer;
    char str[] = "$Hu&n0Q";
    mvstr = mvstr + (pointer->whole_size-8);
    if(strcmp(pointer->secret, str)){
        fprintf(stderr, "MEMORY BUG: %s:%li: detected wild write during free of pointer %p", file, line, ptr);
        has_errored = true;
        return;
    } else {
        for(int i =0; i < 8; i++){
            if(mvstr[i] != str[i]){
                fprintf(stderr, "MEMORY BUG: %s:%li: detected wild write during free of pointer %p", file, line, ptr);
                has_errored = true;
                return;
            }
        }
    }
    statistics.active_size -= pointer->size;
    statistics.nactive--;
    pointer->is_free = true;
    pointer_vals[(long) ptr].is_free = true;
    base_free(pointer);
    return;
    
}

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
void* dcalloc(size_t nmemb, size_t sz, const char* file, long line) {
    // Your code here (to fix test014). 
    if(nmemb*sz > 0xFFFFFFFF || sz > 0xFFFFFFFF || nmemb > 0xFFFFFFFF){
        statistics.nfail++;
        statistics.fail_size += nmemb*sz;
        return nullptr;
    }
    void* ptr = dmalloc(nmemb * sz, file, line);
    if (ptr) {
        memset(ptr, 0, nmemb * sz);
    }
    return ptr;
}

/**
 * get_statistics(stats)
 *      fill a dmalloc_stats pointer with the current memory statistics  
 * 
 * @arg dmalloc_stats *stats : a pointer to the the dmalloc_stats struct we want to fill
 */
void get_statistics(dmalloc_stats* stats) {
    // Stub: set all statistics to enormous numbers
    // memset(stats, 255, sizeof(dmalloc_stats));
    // Your code here.
    
    stats->nactive = statistics.nactive;
    stats->ntotal = statistics.ntotal;
    stats->total_size = statistics.total_size;
    stats->active_size = statistics.active_size;
    stats->nfail = statistics.nfail;
    stats->fail_size = statistics.fail_size;
    stats->heap_min = statistics.heap_min;
    stats->heap_max = statistics.heap_max;
    return;
}

/**
 * print_statistics()
 *      print the current memory statistics to stdout       
 */
void print_statistics() {
    if(has_errored){
        return;
    }
    dmalloc_stats stats;
    get_statistics(&stats);

    printf("alloc count: active %10llu   total %10llu   fail %10llu\n",
           stats.nactive, stats.ntotal, stats.nfail);
    printf("alloc size:  active %10llu   total %10llu   fail %10llu\n",
           stats.active_size, stats.total_size, stats.fail_size);
}

/**  
 * print_leak_report()
 *      Print a report of all currently-active allocated blocks of dynamic
 *      memory.
 */
void print_leak_report() {
    // Your code here.
    if (statistics.nactive > 0){
        for (auto i : pointer_vals) {
            meta_data* pointer = (meta_data*) i.first;
            pointer--;
            if (!(i.second.is_free)){
                printf("LEAK CHECK: %s:%li: allocated object %p with size %li\n", pointer->file_name, pointer->line_number, (void*) i.first, pointer->size);
            }
        }
    }
}
