#include <stdalign.h>

struct buffer {
    // Ordinary pointer to memory. Must be aligned!
    char *buffer;
    // Indexes into this buffer are left shifted by this amount
    int alignment;
    // <<ALIGNMENT total bytes allocated
    unsigned int allocated_space;
    // <<ALIGNMENT current free space pointer for new allocations
    unsigned int free_pointer;
};


/* Initialize an (already allocated) struct buffer. Allocates the actual buffer space.
 * n.b. alignment is the SHIFT size. i.e. 4 for 16 byte, 6 for 64 byte, 10 for 1024, 12 for 4096.
 */
int buffer_init(struct buffer* buf, int alignment, unsigned long long byte_initial_size);
/* Free a buffer
 */
void buffer_destroy(struct buffer* buf);
/* Allocates memory out of a buffer. Return value is error code.
 * All pointers potentially invalidated!!
 */
int buffer_alloc(struct buffer* buf, unsigned long long bytes, void **out_ptr, unsigned int* out_idx);
/* Translates a buffer index into a pointer. May be invalidated by buffer allocation operations!!
 */
void *buffer_index(struct buffer *buf, unsigned int index);


