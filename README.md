expanding-ringbuffer
====================

An auto-expanding circular buffer designed with performance in mind

### Synopsis

```c
# Constructor / Destructor
ringbuf_t *ring = ringbuf_init();
ringbuf_free(ring);

# High-level API
ringbuf_read(ring, out, chunk_size);
ringbuf_write(ring, in, chunk_size);
ringbuf_get_pending_count(ring);

# Low-level API (no extra copyings)
ringbuf_lock(ring);
int actual_size = ringbuf_get_data(ring, expected_size, &start, &end); // mem=O(1), cpu=O(1)
do_stuff();
ringbuf_discard(ring, actual_size);
ringbuf_unlock(ring);
```
