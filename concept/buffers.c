#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "buffers.h"
#include <stdio.h> // debugging printfs. remove

#define public
#define private  static

// Sanity check limit on buffers. Currently 1 TB.
static const unsigned long long MAX_BUFFER_SIZE = 2ull << 40;

private
unsigned long long buffer_compute_size(struct buffer* buf)
{
    assert(buf != NULL);
    assert(buf->buffer != NULL);
    
    // Be sure we convert to ULL before shift!
    return ((unsigned long long)buf->allocated_space) << buf->alignment;
}

private
int buffer_alloc_wholebuffer(struct buffer* buf, unsigned long long byte_initial_size)
{
    assert(buf != NULL);
    assert(buf->buffer == NULL);
    int alignment = buf->alignment;
    
    // This is just a suggestion anyhow.
    if(byte_initial_size < 128) {
        byte_initial_size = 128;
    }

    // Sanity checking. 2 << 40 = 1 TB. Reasonable "something fucked up" number
    assert(byte_initial_size < MAX_BUFFER_SIZE);

    // Size must not be too large for 32bit index at this alignment 
    assert(byte_initial_size >> alignment < 2ull << 32);
    
    size_t space = (byte_initial_size >> alignment) + 1ull;
    
    // TODO: we are not presently ensuring this allocation is aligned, which is problematical :)
    buf->buffer = malloc(space << alignment);
    buf->allocated_space = space;
    buf->free_pointer = 0;
    
    assert(buf->buffer != NULL);
    
    return 0;
}

private
int buffer_realloc_wholebuffer(struct buffer* buf, unsigned long long size)
{
    assert(buf != NULL);
    assert(buf->buffer != NULL);
    int alignment = buf->alignment;

    unsigned long long current_size = buffer_compute_size(buf);
    if(size <= current_size) {
        printf("failure: %llu %llu\n", size, current_size);
    }
    // Always grow
    assert(size > current_size);
    
    // Sanity checking. 2 << 40 = 1 TB. Reasonable "something fucked up" number
    assert(size < MAX_BUFFER_SIZE);

    // Size must not be too large for 32bit index at this alignment 
    unsigned long long space_ull = (size >> alignment) + 1;
    assert(space_ull < 2ull << 32);
    size_t space = (size_t)space_ull;
    
    // TODO: as, above, we're not presently ensuring alignment. problemistical
    char *newbuffer = realloc(buf->buffer, space << alignment);
    
    if(newbuffer == NULL) {
        // TODO: this is maybe not a good think to leave here?
        perror("Reallocating buffer failed");
        return -1;
    }
    
    buf->buffer = newbuffer;
    buf->allocated_space = space;
    // free_pointer remains the same!
    
    return 0;
}


public
int buffer_init(struct buffer* buf, int alignment, unsigned long long byte_initial_size)
{
    assert(buf != NULL);
    // Buggy alignments
    assert(alignment >= 0); // 0 is fine, that's byte-aligned.
    assert(alignment <= 12); // Don't align bigger than 4096.
    
    buf->buffer = 0;
    buf->alignment = alignment;
    buf->allocated_space = 0;
    buf->free_pointer = 0;
    
    return buffer_alloc_wholebuffer(buf, byte_initial_size);
}

public
void buffer_destroy(struct buffer* buf)
{
    assert(buf != NULL);
    // Although free()ing null is okay, this is a programmer error. Don't double free buffers!
    assert(buf->buffer != NULL);
    
    free(buf->buffer);
    buf->buffer = 0;
    buf->alignment = 0;
    buf->allocated_space = 0;
    buf->free_pointer = 0;
}

public
int buffer_alloc(struct buffer* buf, unsigned long long bytes, void **out_ptr, unsigned int* out_idx)
{
    assert(buf != NULL);
    assert(buf->buffer != NULL);
    // We don't consider null allocations acceptable.
    assert(bytes > 0);
    // Highest bit should never be set, usually indicative of overflow or sign extension bug
    assert((bytes & (1ULL << 63)) == 0);
    
    int ret;
    
    int alignment = buf->alignment;
    // mask = e.g. align 3 = 0111, align 5 = 011111, etc.
    unsigned int mask = ((unsigned)-1) >> (32 - alignment);
    unsigned int remainder = bytes & mask;
    
    size_t space = (bytes >> alignment) + (remainder ? 1 : 0);
    
    assert(space > 0);
    
    unsigned int return_idx = buf->free_pointer;
    unsigned int new_fp = return_idx + space;
    
    // Ensure overflows of free_pointer do not happen.
    // Necessary because if it does overflow, we likely will not see the need to realloc,
    // and so no other check will "get in the way" so to speak.
    if(new_fp <= return_idx) {
        // We have run out of our ability to allocate buffers of sufficient size.
        // e.g. align = 0, idx > 4GB, cannot represent this as 32 bit index.
        *out_idx = -1;
        *out_ptr = NULL;
        return -1;
    }
    
    // It's acceptable if new_fp == allocated_space, as that just mean there's 0 free room left.
    if(new_fp > buf->allocated_space) {
        // TODO: better heuristics for expanding allocated buffers?
        // Currently two pieces of info:
        // 1: add the needed allocation. This is IMPORTANT. Don't want to realloc just to realloc again.
        // 2: Multiply by 1.5X, to do modest exponential expansion
        // 3: Add one. I'm paranoid. May not be needed
        unsigned long long newsize = (buffer_compute_size(buf) + bytes) * 1.5 + 1;
        
        ret = buffer_realloc_wholebuffer(buf, newsize);
        
        if(ret != 0) {
            *out_idx = -1; // do not rely on this! note: unsigned, so all ones.
            *out_ptr = NULL; // sure, okay.
            return -1;
        }
    }
    
    buf->free_pointer = new_fp;
    
    *out_idx = return_idx;
    *out_ptr = buffer_index(buf, return_idx);
    
    size_t low_bits_mask = (1ULL << buf->alignment) - 1;
    // TODO: once buffer is aligned correctly, we could assert equal to zero
    assert((((size_t)*out_ptr) & low_bits_mask) == (((size_t)buf->buffer) & low_bits_mask));
    
    assert((char*)*out_ptr >= buf->buffer);
    
    return 0;
}

public
void *buffer_index(struct buffer *buf, unsigned int index)
{
    assert(buf != NULL);
    assert(buf->buffer != NULL);
    assert(index < buf->free_pointer);
    return buf->buffer + (((unsigned long long)index) << buf->alignment);
}




