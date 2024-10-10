#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdarg.h>

#include "../io300.h"


/*
    student.c
    Fill in the following stencils
*/

/*
    When starting, you might want to change this for testing on small files.
*/
#ifndef CACHE_SIZE
#define CACHE_SIZE 100
#endif

#if(CACHE_SIZE < 4)
#error "internal cache size should not be below 4."
#error "if you changed this during testing, that is fine."
#error "when handing in, make sure it is reset to the provided value"
#error "if this is not done, the autograder will not run"
#endif

/*
   This macro enables/disables the dbg() function. Use it to silence your
   debugging info.
   Use the dbg() function instead of printf debugging if you don't want to
   hunt down 30 printfs when you want to hand in
*/
#define DEBUG_PRINT 0
#define DEBUG_STATISTICS 1

struct io300_file {
    /* read,write,seek all take a file descriptor as a parameter */
    int fd;
    /* this will serve as our cache */
    char *cache;

    // TODO: Your properties go here
    size_t cachehead;
    size_t filehead;
    size_t filenum[2];
    int has_changed;
    ssize_t cachebytes;
    ssize_t is_end;

    /* Used for debugging, keep track of which io300_file is which */
    char *description;
    /* To tell if we are getting the performance we are expecting */
    struct io300_statistics {
        int read_calls;
        int write_calls;
        int seeks;
    } stats;
};

/*
    Assert the properties that you would like your file to have at all times.
    Call this function frequently (like at the beginning of each function) to
    catch logical errors early on in development.
*/
static void check_invariants(struct io300_file *f) {
    assert(f != NULL);
    assert(f->cache != NULL);
    assert(f->fd >= 0);

    // TODO: Add more invariants
}

/*
    Wrapper around printf that provides information about the
    given file. You can silence this function with the DEBUG_PRINT macro.
*/
static void dbg(struct io300_file *f, char *fmt, ...) {
    (void)f; (void)fmt;
#if(DEBUG_PRINT == 1)
    static char buff[300];
    size_t const size = sizeof(buff);
    int n = snprintf(
        buff,
        size,
        // TODO: Add the fields you want to print when debugging
        "{desc:%s, } -- ",
        f->description
    );
    int const bytes_left = size - n;
    va_list args;
    va_start(args, fmt);
    vsnprintf(&buff[n], bytes_left, fmt, args);
    va_end(args);
    printf("%s", buff);
#endif
}

int prefetch(struct io300_file *const f){
    // lseek(f->fd, f->filehead, SEEK_SET);
    // if(f->cachebytes != 0 && f->cachebytes > CACHE_SIZE){
    //     return f->cachebytes;
    // }
    f->cachebytes = read(f->fd, f->cache, CACHE_SIZE);
    if(f->cachebytes == -1){
        fprintf(stderr, "error: could not read correctly\n");
        return -1;
    }
    f->stats.read_calls++;
    if(f->cachebytes == 0){
        f->is_end = 0;
    } else if(f->cachebytes < CACHE_SIZE){
        f->is_end = f->cachebytes; //Sets the end index of the cache if the cache is bigger than the file
    } else {
        f->is_end = -1;
    }

    f->filenum[0] = f->filehead;
    //This logic doesn't make any sense if you're writing to a file that is different from the current one and the written one has no bytes.
    if(f->cachebytes > 0){
        f->filenum[1] = f->filehead + f->cachebytes -1;
    } else {
        f->filenum[1] = f->filehead;
    }
    f->cachehead = 0;
    return f->cachebytes;
}

struct io300_file *io300_open(const char *const path, char *description) {
    if (path == NULL) {
        fprintf(stderr, "error: null file path\n");
        return NULL;
    }

    int const fd = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        fprintf(stderr, "error: could not open file: `%s`: %s\n", path, strerror(errno));
        return NULL;
    }

    struct io300_file *const ret = malloc(sizeof(*ret));
    if (ret == NULL) {
        fprintf(stderr, "error: could not allocate io300_file\n");
        return NULL;
    }

    ret->fd = fd;
    ret->cache = malloc(CACHE_SIZE);
    if (ret->cache == NULL) {
        fprintf(stderr, "error: could not allocate file cache\n");
        close(ret->fd);
        free(ret);
        return NULL;
    }
    ret->stats.read_calls = 0;
    ret->stats.write_calls = 0;
    ret->stats.seeks = 0;
    ret->description = description;
    // TODO: Initialize your file
    ret->filehead = 0;
    ret->has_changed = 0;
    ssize_t output = prefetch(ret);
    if(output == -1) {
        fprintf(stderr, "error: could not prefetch data\n");
        return ret;
    }
    check_invariants(ret);
    dbg(ret, "Just finished initializing file from path: %s\n", path);
    return ret;
}

int io300_seek(struct io300_file *const f, off_t const pos) {
    //Advise not prefetching within seek -> need to keep track of the pointers
    //Running gdb with block_cat_1 and then two test files -> shrink the cache to 8 bytes -> that's a good testing number
    check_invariants(f);
    f->stats.seeks++;
    ssize_t output_num2;
    if((size_t) pos >=  f->filenum[0] && (size_t) pos <= f->filenum[1]){
        f->cachehead += (pos - f->filehead);
        f->filehead = pos;
    }
    io300_flush(f);
    output_num2 = lseek(f->fd, pos, SEEK_SET);
    prefetch(f);
    // TODO: Implement this
    return output_num2;
}

int io300_close(struct io300_file *const f) {
    check_invariants(f);
    if(f->has_changed){
            io300_flush(f);
        }
#if(DEBUG_STATISTICS == 1)
    printf("stats: {desc: %s, read_calls: %d, write_calls: %d, seeks: %d}\n",
            f->description, f->stats.read_calls, f->stats.write_calls, f->stats.seeks);
#endif
    // TODO: Implement this
    
    close(f->fd);
    free(f->cache);
    free(f);
    return 0;
}

off_t io300_filesize(struct io300_file *const f) {
    check_invariants(f);
    struct stat s;
    int const r = fstat(f->fd, &s);
    if (r >= 0 && S_ISREG(s.st_mode)) {
        return s.st_size;
    } else {
        return -1;
    }
}



int io300_readc(struct io300_file *const f) {
    check_invariants(f);
    // TODO: Implement thisx
    unsigned char c;
    ssize_t output_num2;
    if(f->filehead <= f->filenum[1]) {
        c = f->cache[f->cachehead];
        f->cachehead++;
        f->filehead++;
        return c;
    } else if (f->is_end != -1 && f->filehead > (size_t) f->is_end) {
        return -1;
    }
    else {
        io300_flush(f);
        output_num2 = prefetch(f);
        if(output_num2 == -1){
            fprintf(stderr, "error: could not prefetch data\n");
            return -1;        
        }
        c = f->cache[f->cachehead];
        f->cachehead++;
        f->filehead++;
        return c;
    }
  
}
int io300_writec(struct io300_file *f, int ch) {
    check_invariants(f);
    unsigned char const c = (char)ch;
    ssize_t output_num2;
    // ssize_t ret = 0;

    // // TODO: Implement this
    // //For write if filehead is less it's okay to still call and write to the cache is filenum[1] < CACHE_SIZE
    // // Need to make this one a little more efficient so it writes to cache intead of repeatedly calling read/write calls.
    // // Need to put multiple cases for when the file is empty and for the ROT 13 test
    // if(f->filehead <= f->filenum[1] && f->cachebytes < CACHE_SIZE) {
    //     // if(f->cache[f->cachehead] != c){
                 
    //     // }
    //      f->cache[f->cachehead] = c;
    //      f->has_changed = 1;  
    //     // f->cache[f->cachehead] = c;
    //     // f->has_changed = 1;
    //     if(f->filehead == f->filenum[1]){
    //         f->cachebytes++;
    //         f->filenum[1] += 1;
    //     }
    //     f->cachehead++;
    //     f->filehead++;
    //     //Double check this case

    //     return c;
    // } else if (f->filehead <= f->filenum[1] && f->cachebytes == CACHE_SIZE && (ssize_t) f->cachehead < f->cachebytes){
    //     // if(f->cache[f->cachehead] != c){
                  
    //     // } 
    //     f->cache[f->cachehead] = c;
    //     f->has_changed = 1;        
    //     f->cachehead++;
    //     f->filehead++;
    //     //Double check this case
    //     return c;
    // }
    // else {
    //     io300_flush(f);
    //     output_num2 = prefetch(f);
    //     if(output_num2 == -1){ 
    //         fprintf(stderr, "error: could not prefetch data\n");
    //         return -1;        
    //     }
    //     *(f->cache+f->cachehead) = c; //This is wrong
    //     f->has_changed = 1;
    //     f->cachehead++;
    //     f->filehead++;
    //     if(f->cachebytes == 0){
    //         f->cachebytes++;
    //     }
    //     return c;
    // }
    // return c;
    //For write if filehead is less it's okay to still call and write to the cache is filenum[1] < CACHE_SIZE
    // Need to make this one a little more efficient so it writes to cache intead of repeatedly calling read/write calls.
    if(f->filehead <= f->filenum[1] && f->cachebytes < CACHE_SIZE) {
        // printf("the if statement\n");

        f->cache[f->cachehead] = c;
        f->has_changed = 1;
        f->cachehead++;
        f->filehead++;
        f->filenum[1]++;
        f->cachebytes++;
        return c;
    } 
    else {
        // printf("the else statement\n");
        io300_flush(f);
        output_num2 = prefetch(f);
        if(output_num2 == -1){ 
            fprintf(stderr, "error: could not prefetch data\n");
            return -1;        
        }
        *(f->cache+f->cachehead) = c; //This is wrong
        f->has_changed = 1;
        if(f->filehead == f->filenum[1]){
            f->filenum[1]++;
        }
        f->cachehead++;
        f->filehead++;
        if(f->cachebytes == 0){
            f->cachebytes++;
        }
        return c;
    }

}

ssize_t io300_read(struct io300_file *const f, char *const buff, size_t const sz) {
    check_invariants(f);
    // TODO: Implement this

    //Multiple Cases
    // Case #1 -> at the end of the file -> can't read anymore bytes
    // Case #2 -> close to the end of the file -> read all the bytes from the rest of the cache
    // Case #3 -> All bytes are in the cache -> copy them from cache to buff and then update the pointers
    // Case #4 -> Some bytes are in the cache -> read those bytes into buff, then flush, then read rest into buff
    // Case #5 -> No bytes are inside the cache -> could copy bytes directly from read to buff, then prefetch more after
    if(f->is_end != -1 && (f->cachehead == (size_t) f->is_end)) { // This means that it is at the end of the file -> cachehead and this will only equal at this
        return 0;
    } else if (f->is_end != -1 && sz >= (f->is_end - f->cachehead)){ // Size is larger than what is remaining in the file
        memcpy(buff, &(f->cache[f->cachehead]), (f->is_end - f->cachehead)); //Will this implementation work for memcpy -> technically you're passing in an address right?
        ssize_t data_return = f->is_end - f->cachehead;
        f->cachehead = f->is_end;
        f->filehead = f->filenum[1]+1;
        return data_return;
    } else if ((f->filehead + sz) <= (f->filenum[1]+1)){ // Why do we need sz-1 (???)
        memcpy(buff, &(f->cache[f->cachehead]), sz); // is this sz-1 (???) or sz (???)
        f->cachehead += sz;
        f->filehead += sz;
        return sz;
    } else if (f->filehead <= f->filenum[1]){
        ssize_t bytesread = (f->filenum[1] - f->filehead) +1;
        ssize_t bytesleft = sz - bytesread;
        memcpy(buff, &(f->cache[f->cachehead]), bytesread);
        f->filehead += bytesread;
        f->cachehead += bytesread;
        // Can change this to just reading in and out of a file directly -> should solve the problems
        while(bytesleft > 0){
            io300_flush(f);
            prefetch(f);
            if(bytesleft > CACHE_SIZE && f->is_end == -1){
                memcpy(&(buff[bytesread]), &(f->cache[f->cachehead]), CACHE_SIZE);
                bytesread += CACHE_SIZE;
                bytesleft -= CACHE_SIZE;
                f->filehead += CACHE_SIZE;
                f->cachehead += CACHE_SIZE;
            } else if (bytesleft > CACHE_SIZE && f->is_end != -1){
                memcpy(&(buff[bytesread]), &(f->cache[f->cachehead]), f->cachebytes);
                bytesread += f->cachebytes;
                bytesleft -= f->cachebytes;
                f->filehead += f->cachebytes;
                f->cachehead += f->cachebytes;
                return bytesread;
            } else {
                memcpy(&(buff[bytesread]), &(f->cache[f->cachehead]), bytesleft);
                f->filehead += bytesleft;
                f->cachehead += bytesleft;
                bytesread += bytesleft;
                bytesleft -= bytesleft;
                return bytesread;
            }
        }
        return bytesread;
    } else { // no data is in the cache that is being copied
        io300_flush(f);
        ssize_t readbytes = read(f->fd, buff, sz); // Why am I reading here and not calling prefetch ((????))
        if (readbytes == -1){
            fprintf(stderr, "error: could not read correctly\n");
            return -1;
        }
        f->stats.read_calls++;
        if((size_t) readbytes < sz){ //read the rest of the file into the buffer
            f->filehead += readbytes;
            f->is_end = 0; // the cache is empty since it was just flushed
            f->filenum[0] = f->filehead;
            f->filenum[1] = f->filehead;
            f->cachehead = 0;
            //Need to figure out what to do -> !!! Program needs to know when a file cannot be read anymore with some variable logic.
        } else {
            f->filehead += sz;
            prefetch(f);
        }
        return readbytes;
    }

    // return read(f->fd, buff, sz);
    return -1;
}
ssize_t io300_write(struct io300_file *const f, const char *buff, size_t const sz) {
    check_invariants(f);
    // TODO: Implement this

    //Multiple Cases
    // Case #1 -> the write will be fully in the cache -> just need to memcpy and increment all the meta_data
    // Case #2 -> Write crosses cache boundary -> write however much you can into cache -> flush -> write rest into cache -> need to do an extra seek
    // Case #3 -> If write is larger than the cache size, then you can directly update the buffer, and update the meta data accordingly
    // Case #4 -> Write is outside of cache (similar to crossing boundary above -> need case for that)...
    if (sz >= CACHE_SIZE){
        size_t copy = f->filehead;
        int x = io300_flush(f);
        if (x==1) {
            lseek(f->fd, copy, SEEK_SET);
        }
        f->stats.seeks++;        
        // size_t fz = io300_filesize(f);
        ssize_t wr = write(f->fd, buff, sz);
        f->stats.write_calls++;
        f->filehead += sz;
        if(wr == -1){
            fprintf(stderr, "error: could not write correctly\n");
        }

    } else if(f->filehead == f->filenum[1] && f->cachebytes < CACHE_SIZE && (ssize_t) sz  <= (CACHE_SIZE - f->cachebytes)){ // filenum1 sometimes not correct value
        memcpy(&(f->cache[f->cachehead]), buff, sz);
        f->filehead += sz;
        f->cachehead += sz;
        f->cachebytes += sz;
        f->filenum[1] +=sz;
        f->has_changed = 1;
    }
    else if(f->filehead + sz <= f->filenum[1]) { // If it's inside the cache -> this will **ONLY** work for files which are both read/write. Not needed if you're writing into the other file
        memcpy(&(f->cache[f->cachehead]), buff, sz);
        f->filehead += sz;
        f->cachehead += sz;
        f->has_changed = 1;
    } 
    // else if (sz >= CACHE_SIZE){ // If it's bigger than the cache
        // size_t copy = f->filehead;
        // int x = io300_flush(f);
        // if (x==1) {
        //     lseek(f->fd, copy, SEEK_SET);
        // }
        // f->stats.seeks++;        
        // // size_t fz = io300_filesize(f);
        // ssize_t wr = write(f->fd, buff, sz);
        // f->stats.write_calls++;
        // f->filehead += sz;
        // if(wr == -1){
        //     fprintf(stderr, "error: could not write correctly\n");
        // }
        // if(f->filehead > fz) { // if the entire header is now past the old end of file
        //     f->filenum[1] = f->filehead-1;
        //     f->is_end = 0; // Has ending
        // } else {
        //     prefetch(f);
        // }
    // } 
    else if (f->filehead < f->filenum[1]){ // If it's partially in the cache -> I need another case if they are zero or equal
        size_t byteswritten = (f->filenum[1] - f->filehead) +1;
        size_t bytesleft = sz - byteswritten;
        memcpy(&(f->cache[f->cachehead]), buff, byteswritten);
        //Need to update f->filenum[1] or ??? the problem is this will always flush because it's saying there's 0
        f->filehead += byteswritten;
        f->cachehead += byteswritten;
        f->has_changed = 1;
        io300_flush(f);
        size_t fz = io300_filesize(f);
        // The bytes that are in the cache are updated and written to the file.
        write(f->fd, &(buff[byteswritten]), bytesleft); // Don't need to seek because we know from the above calculation that it hit the end of the cache.
        f->stats.write_calls++;
        f->filehead += bytesleft;
        f->cachehead += bytesleft;
        if(f->filehead > fz) { // if the entire header is now past the old end of file
            f->filenum[1] = f->filehead-1;
            f->is_end = 0; // Has ending
        } else {
            prefetch(f);
        }
    } else { // If it is completely outside of the cache -> prefetch more data -> write to that data -> this is *not* bigger than the cache.
        io300_flush(f);
        // prefetch(f);
        // lseek(f->fd, f->filehead, SEEK_SET);
        // f->stats.seeks++;
        f->cachehead = 0;
        f->filenum[0] = f->filehead;
        memcpy(&(f->cache[f->cachehead]), buff, sz);
        f->has_changed = 1;
        f->filehead += sz;
        f->cachehead += sz;
        f->filenum[1] = f->filehead;
        f->cachebytes = sz;
    }
    return sz;
}

int io300_flush(struct io300_file *const f) { // wherever flush is called, refetch has to be called right after, except for the last function
    check_invariants(f);
    if(f->has_changed){
        lseek(f->fd, f->filenum[0], SEEK_SET);
        write(f->fd, f->cache, f->cachebytes);
        f->stats.write_calls++;
        f->stats.seeks++;
        f->has_changed = 0;
        return 1;
    }
    return 0;
}