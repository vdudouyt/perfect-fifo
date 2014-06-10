#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ringbuf.h>

int run_secs = 10;

struct {
	unsigned int bytes_read;
	unsigned int bytes_written;
} stats;

void show_status()
{
}

void *worker( ringbuf_t *ring )
{
    unsigned int chunk_size = 30 * pow(2,20);
    char *buf1 = malloc(chunk_size);
    char *buf2 = malloc(chunk_size);
    memset(buf1, 0x05, chunk_size);
    while(1) {
        stats.bytes_written += ringbuf_write(ring, buf1, chunk_size);
        //stats.bytes_read += ringbuf_read(ring, buf2, chunk_size);
        printf("stats.bytes_read = %u, stats.bytes_written = %u, size=%u\n", stats.bytes_read, stats.bytes_written, ring->size);
    }
    free(buf1);
}

int main()
{
    memset(&stats, 0, sizeof(stats));
    ringbuf_t *ring = ringbuf_init();
    worker(ring);
    ringbuf_free(ring);
    exit(0);
}
