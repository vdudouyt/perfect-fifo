#include <stdio.h>
#include <stdlib.h>
#include <check.h>
#include "../ringbuf.h"

#define INIT_RING_BUFFER ringbuf_t *ring = ringbuf_init(); ringbuf_reset(ring, 10);

char *get_test_string(int length)
{
    static char data[256];
    int i;
    for(i = 0; i < length; i++)
    {
        data[i] = 0x60 + i;
    }
    data[i] = '\0';
    return data;
}

START_TEST (test_lowlevel)
{
    INIT_RING_BUFFER
    char *str1 = get_test_string(15);
    ring->read_ptr = 7;
    ck_assert_int_eq(ringbuf_chunk_looking_at(ring), 3);
    ck_assert_int_eq(ringbuf_discard(ring, 100), 3);
    ck_assert_int_eq(ring->read_ptr, 0);

    ring->write_ptr = 5;
    ck_assert_int_eq(ringbuf_chunk_looking_at(ring), 5);

    ringbuf_expand(ring, 25);
    ck_assert_int_eq(ring->size, 30);

    // 0 <= p1 <= read_ptr <= p0 <= write_ptr <= size
    // Read order: chunk1 = (read_ptr, p0), chunk2 = (0, p1), chunk3 = (p0, write_ptr)
    ring->write_ptr = 5;
    ring->read_ptr = 10;
    ck_assert_int_eq(ringbuf_chunk_looking_at(ring), 20);
    ring->p0 = 20;
    ring->p1 = 5;
    ring->write_ptr = 28;
    // chunk 1
    ck_assert_int_eq(ringbuf_chunk_looking_at(ring), 10);
    ck_assert_int_eq(ringbuf_discard(ring, 100), 10);
    ck_assert_int_eq(ring->read_ptr, 0);
    // chunk 2
    ck_assert_int_eq(ringbuf_chunk_looking_at(ring), 5);
    ck_assert_int_eq(ringbuf_discard(ring, 100), 5);
    ck_assert_int_eq(ring->read_ptr, 20);
    ck_assert_int_eq(ring->p0, 0);
    // chunk 3
    ck_assert_int_eq(ringbuf_chunk_looking_at(ring), 8);
    ck_assert_int_eq(ringbuf_discard(ring, 100), 8);
    ck_assert_int_eq(ring->read_ptr, 28);
    ck_assert_int_eq(ringbuf_chunk_looking_at(ring), 0);
    ringbuf_free(ring);
}
END_TEST

START_TEST (test_read_write)
{
    INIT_RING_BUFFER
    char *str1 = get_test_string(5);
    ck_assert_int_eq(strlen(str1), 5);

    int bytes_written = ringbuf_write(ring, str1, 5);
    ck_assert_int_eq(bytes_written, 5);
    ck_assert_int_eq(ringbuf_chunk_looking_at(ring), bytes_written);

    char str2[5];
    int bytes_read = ringbuf_read(ring, str2, 10);
    ck_assert_int_eq(bytes_read, bytes_written);
    ck_assert_int_eq(ring->write_ptr, ring->read_ptr);
    ck_assert_str_eq(str1, str2);
    ringbuf_free(ring);
}
END_TEST

START_TEST (test_simple_expand)
{
    INIT_RING_BUFFER
    char *str1 = get_test_string(13);
    ringbuf_write(ring, str1, 14);
    ck_assert_int_eq(ring->size, 20);
    ringbuf_write(ring, str1, 14);
    ck_assert_int_eq(ring->size, 30);

    char str2[28];
    ck_assert(ringbuf_read(ring, str2, 100) == 28);
    ck_assert_str_eq(str1, str2);
    ck_assert_str_eq(str1, str2 + 14);
}
END_TEST

START_TEST (test_circulating)
{
    INIT_RING_BUFFER
    char *str1 = get_test_string(2);
    char str2[3];
    int i;
    for(i = 0; i < 50; i++)
    {
        memset(str2, 0, sizeof(str2));
        ck_assert_int_eq(ring->size, 10);
        ck_assert_int_lt(ring->read_ptr, ring->size);
        ck_assert_int_lt(ring->write_ptr, ring->size);
        ck_assert_int_eq(ringbuf_write(ring, str1, 3), 3);
        ck_assert_int_eq(ringbuf_read(ring, str2, 100), 3);
        ck_assert_str_eq(str1, str2);
    }
    ringbuf_free(ring);
}
END_TEST

START_TEST (test_tricky_expand)
{
    INIT_RING_BUFFER
    char *str1 = get_test_string(14);
    char str2[22];
    char str3[] = "`abcdef`abcdefghijklm";

    ringbuf_write(ring, str1, 7);
    ringbuf_read(ring, str2, 100);
    ringbuf_write(ring, str1, 7);
    ck_assert_int_eq(ring->size, 10);
    ck_assert_int_lt(ring->write_ptr, ring->read_ptr);
    ck_assert_int_eq(ring->write_ptr, 4);
    ck_assert_int_eq(ring->read_ptr, 7);

    ringbuf_write(ring, str1, 15);
    ck_assert_int_eq(ring->p0, 10);
    ck_assert_int_eq(ring->p1, 7);
    ck_assert_int_eq(ring->size, 30);
    ck_assert_int_eq(ringbuf_read(ring, str2, 100), 22);
    ck_assert_str_eq(str3, str2);
}
END_TEST

START_TEST (test_expand_by_1)
{
    INIT_RING_BUFFER
    char *str1 = get_test_string(13);
    int i = 0;
    for(i = 1; i < 10; i++)
    {
	    ringbuf_write(ring, str1, 9);
	    ck_assert_int_eq(ring->size, 10 * i);
    }
}
END_TEST

/* Usual boilerplate */
Suite *ringbuf_suite()
{
    Suite *s = suite_create ("Ring Buffer");
    TCase *tc_core = tcase_create ("Core");
    tcase_add_test (tc_core, test_lowlevel);
    tcase_add_test (tc_core, test_read_write);
    tcase_add_test (tc_core, test_simple_expand);
    tcase_add_test (tc_core, test_circulating);
    tcase_add_test (tc_core, test_tricky_expand);
    tcase_add_test (tc_core, test_expand_by_1);
    suite_add_tcase (s, tc_core);
    return s;
}

int main()
{
    int number_failed;
    Suite *s = ringbuf_suite();
    SRunner *sr = srunner_create (s);
    srunner_run_all (sr, CK_NORMAL);

    srunner_free(sr);
    return 0;
}
