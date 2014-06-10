.PHONY: all clean test stress-test
SOURCES=ringbuf.c
TESTS=tests/test_ringbuf.c
OBJECTS=$(SOURCES:.c=.o)
TEST_OBJECTS=$(TESTS:.c=.o)

CFLAGS=`pkg-config --cflags --libs glib-2.0` -g -O0
LDLAGS=`pkg-config --libs --libs glib-2.0`

all: $(OBJECTS)

clean:
	rm -f $(OBJECTS) $(TEST_OBJECTS) $(PROG)

test: $(OBJECTS) $(TEST_OBJECTS)
	@gcc ringbuf.o $(LDLAGS) $(TEST_OBJECTS) `pkg-config --cflags --libs check` -o test
	@./test

stress: $(OBJECTS)
	@gcc ringbuf.o stress-testing/test_ringbuf.c $(CFLAGS) $(LDLAGS) -o stress -I.
	./stress
