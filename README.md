expanding-ringbuffer
====================

A high-performance auto-expanding circular buffer

### Synopsis

```c
# Initializators
ringbuf_t *ring = ringbuf_init();
ringbuf_free(ring);

# High-level API
ringbuf_read(ring, out, chunk_size);
ringbuf_write(ring, in, chunk_size);
ringbuf_get_pending_count(ring);

# Low-level API
ringbuf_lock(ring);
ringbuf_get_data(ring, chunk_size, &start, &end);
do_stuff();
ringbuf_discard(ring, chunk_size);
ringbuf_unlock(ring);
```
