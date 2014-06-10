#ifndef __RINGBUF_H
#define __RINGBUF_H

typedef struct ringbuf {
    unsigned char *data;
    unsigned int read_ptr; // last byte readed
    unsigned int write_ptr; // last byte written
    unsigned int size; // current size of the buffer
    unsigned int step; // growing step
    unsigned int p0; // where last+1 was when expand occurred
    unsigned int p1; // where write_ptr was when expand occurred
    FILE *fd; // for using with select()
} ringbuf_t;

ringbuf_t *ringbuf_init();
void ringbuf_free(ringbuf_t *ring);

int ringbuf_chunk_looking_at(ringbuf_t *ring);
int ringbuf_get_data(ringbuf_t *ring, unsigned int expected_size, unsigned char **start, unsigned char **end);
int ringbuf_discard(ringbuf_t *ring, unsigned int size);
void ringbuf_set_step(ringbuf_t *ring, unsigned int step);
int ringbuf_read(ringbuf_t *ring, unsigned char *buf, unsigned int size);
int ringbuf_write(ringbuf_t *ring, unsigned char *buf, unsigned int write_size);

#endif
