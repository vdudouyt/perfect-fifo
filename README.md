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
### API
* int ringbuf_get_data(ringbuf_t *ring, unsigned int expected_size, unsigned char **start, unsigned char **end);

  Obtains a direct access to the start of the buffer. Can return less bytes than 'expected_size' due to the internal structure.
  Doesn't modifies anything, so you're expected to use ringbuf_discard() in otder to move forward.
  Doesn't copies anything, so it's guaranteed to be O(1) of memory and O(1) of CPU.
  Returns number of the bytes in block.

* int ringbuf_discard(ringbuf_t *ring, unsigned int size);

  Discards 'size' of bytes from the front of ringbuf. May discat
  Moves the read index forward.
  Costs O(1) of memory and O(1) of CPU time.
  Returns number of the bytes discarded.

*  int ringbuf_read(ringbuf_t *ring, unsigned char *buf, unsigned int size);  

  Reads 'size' bytes from the start of buffer

*  int ringbuf_write(ringbuf_t *ring, unsigned char *buf, unsigned int write_size);

  Writes 'size' bytes to the end of buffer
