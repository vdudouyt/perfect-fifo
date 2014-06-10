#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "ringbuf.h"

#define ALLOC_STEP (50 * pow(2, 20))
#define MIN(a, b) ((a) > (b) ? b : a)
#define MIN3(a, b, c) (MIN((a), (b)) > (c) ? MIN((a), (b)) : c)
#define MAX(a, b) ((a) > (b) ? a : b)

#define READ_PTR ring->read_ptr
#define WRITE_PTR ring->write_ptr

ringbuf_t *ringbuf_init()
{
    ringbuf_t *ret = malloc(sizeof(ringbuf_t));
    memset(ret, 0, sizeof(ringbuf_t));
    ret->step = ret->size = ALLOC_STEP;
    ret->data = malloc(ret->size);
    return(ret);
}

void ringbuf_free(ringbuf_t *ring)
{
    free(ring->data);
    free(ring);
}

void ringbuf_reset(ringbuf_t *ring, unsigned int step)
{
    ring->size = ring->step = step;
}

void ringbuf_expand(ringbuf_t *ring, int new_size)
{
    ring->size = (1 + new_size / ring->step) * ring->step;
    ring->data = realloc(ring->data, ring->size);
}

void ringbuf_dump(ringbuf_t *ring)
{
    int i;
    printf("sz=%u read_ptr=%u write_ptr=%u p0=%u p1=%u\t|", ring->size, ring->read_ptr, ring->write_ptr, ring->p0, ring->p1);
    for(i = 0; i < ring->size; i++)
    {
        printf("%c", isprint(ring->data[i]) ? ring->data[i] : '.');
    }
    printf("\n");
}

unsigned int ringbuf_get_pending_count(ringbuf_t *ring)
{
    if(ring->write_ptr >= ring->read_ptr)
        return ring->write_ptr - ring->read_ptr;
    else
        return ring->size - ring->read_ptr + ring->write_ptr;
}

int ringbuf_lock(ringbuf_t *ring, unsigned int chunk_size)
{
    // TODO
}

int ringbuf_unlock(ringbuf_t *ring, unsigned int chunk_size)
{
    // TODO
}

int ringbuf_chunk_looking_at(ringbuf_t *ring)
{
    if(READ_PTR <= WRITE_PTR && !ring->p0) return (WRITE_PTR - READ_PTR);
    else if(WRITE_PTR < READ_PTR && !ring->p0) return (ring->size - READ_PTR);
    else if(WRITE_PTR > READ_PTR && ring->p0 && READ_PTR >= ring->p1) return (ring->p0 - READ_PTR);
    else if(WRITE_PTR > READ_PTR && ring->p1 > READ_PTR) return (ring->p1 - READ_PTR);
    else assert(0);
}

int ringbuf_get_data(ringbuf_t *ring, unsigned int max_size, unsigned char **start, unsigned char **end)
{
    *start = ring->data + ring->read_ptr;
    *end = ring->data + ring->read_ptr + MIN(ringbuf_chunk_looking_at(ring), max_size);
    return *end - *start;
}

int ringbuf_discard(ringbuf_t *ring, unsigned int max_size)
{
    int bytes_read = MIN(ringbuf_chunk_looking_at(ring), max_size);
    ring->read_ptr = (ring->read_ptr + bytes_read) % ring->size;
    if(WRITE_PTR > READ_PTR && ring->p0 && ring->read_ptr >= ring->p0) ring->read_ptr -= ring->p0;
    if(WRITE_PTR >= READ_PTR && ring->p1 && ring->read_ptr >= ring->p1)
    {
        ring->read_ptr = ring->p0 + (ring->read_ptr - ring->p1);
        ring->p0 = ring->p1 = 0;
    }
    return bytes_read;
}

int ringbuf_read(ringbuf_t *ring, unsigned char *buf, unsigned int size)
{
    int chunk_size, bytes_read = 0;
    unsigned char *start, *end;
    while(ringbuf_chunk_looking_at(ring) > 0)
    {
        int actual_size = ringbuf_get_data(ring, size, &start, &end);
        memcpy(buf + bytes_read, start, end - start);
        ringbuf_discard(ring, actual_size);
        bytes_read += actual_size;
    }
    return bytes_read;
}

int ringbuf_write(ringbuf_t *ring, unsigned char *buf, unsigned int write_size)
{
    int p2 = WRITE_PTR + write_size; // Rightmost index which ringbuf_write() is challenging for + 1
    int p3 = p2 - WRITE_PTR; // Same but after carry occurred
    if(READ_PTR <= WRITE_PTR && p2 <= ring->size)
    {
        memcpy(ring->data + WRITE_PTR, buf, write_size);
        WRITE_PTR = (WRITE_PTR + write_size) % ring->size;
        return write_size;
    }
    else if(READ_PTR <= WRITE_PTR && !ring->p0 && p2 > ring->size && p3 <= READ_PTR)
    {
        // Carry tail to front
        int size1 = ring->size - WRITE_PTR;
        int size2 = p2 - ring->size;
        assert(size1 + size2 == write_size);
        ringbuf_write(ring, buf, size1);
        memcpy(ring->data, buf + size1, size2);
        WRITE_PTR += size2;
        return(size1 + size2);
    }
    else if(READ_PTR <= WRITE_PTR && !ring->p0 && p2 > ring->size && p3 > READ_PTR)
    {
        ringbuf_expand(ring, p2);
        return ringbuf_write(ring, buf, write_size);
    }
    else if(WRITE_PTR < READ_PTR && !ring->p0 && p2 <= READ_PTR)
    {
        memcpy(ring->data + WRITE_PTR, buf, write_size);
        WRITE_PTR = WRITE_PTR + write_size;
        return write_size;
    }
    else if(WRITE_PTR < READ_PTR && !ring->p0 && p2 > ring->size && p3 > READ_PTR)
    {
        int size1 = READ_PTR - WRITE_PTR;
        int size2 = write_size - size1;
        ringbuf_write(ring, buf, size1);
        ring->p0 = ring->size;
        ring->p1 = WRITE_PTR;
        WRITE_PTR = ring->p0;
        ringbuf_expand(ring, ring->size + size2);
        assert(READ_PTR < WRITE_PTR);
        return ringbuf_write(ring, buf + size1, size2);
    }
    else { assert(0); }
}
