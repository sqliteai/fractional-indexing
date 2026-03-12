//
//  main.c
//  fractional_indexing
//
//  Created by Marco Bambini on 26/08/23.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fractional_indexing.h"

typedef char *midpoint_callback (const char *a, const char *b, char **error);
typedef char *incdec_callback (const char *a);
typedef char *keygen_callback (const char *a, const char *b);
typedef char **nkeygen_callback (const char *a, const char *b, int n);

static int total_count = 0;
static int total_success = 0;
static int total_failure = 0;

#define START_TEST()                int ncount = 0; int nsuccess = 0; int nfailure = 0;
#define END_TEST()                  do { \
    printf("\nTest counter: %d\nSuccesses: %d\nFailures: %d\nSuccess Rate: %.2f%%\n\n", \
           ncount, nsuccess, nfailure, (double)(nsuccess * 100.0 / ncount)); \
    total_count += ncount; total_success += nsuccess; total_failure += nfailure; \
} while(0)

#define TEST_BASE10(_a, _b, _res)   do_test_midpoint(midpoint_base10, _a, _b, _res, ++ncount, &nsuccess, &nfailure)
#define TEST_BASE62(_a, _b, _res)   do_test_midpoint(midpoint_base62, _a, _b, _res, ++ncount, &nsuccess, &nfailure)
#define TEST_BASE95(_a, _b, _res)   do_test_midpoint(midpoint_base95, _a, _b, _res, ++ncount, &nsuccess, &nfailure)

#define TEST_INC10(_a, _b)          do_test_incdec(increment_integer_base10, _a, _b, ++ncount, &nsuccess, &nfailure)
#define TEST_INC62(_a, _b)          do_test_incdec(increment_integer_base62, _a, _b, ++ncount, &nsuccess, &nfailure)
#define TEST_INC95(_a, _b)          do_test_incdec(increment_integer_base95, _a, _b, ++ncount, &nsuccess, &nfailure)

#define TEST_DEC10(_a, _b)          do_test_incdec(decrement_integer_base10, _a, _b, ++ncount, &nsuccess, &nfailure)
#define TEST_DEC62(_a, _b)          do_test_incdec(decrement_integer_base62, _a, _b, ++ncount, &nsuccess, &nfailure)
#define TEST_DEC95(_a, _b)          do_test_incdec(decrement_integer_base95, _a, _b, ++ncount, &nsuccess, &nfailure)

#define TEST_KEY10(_a, _b, _res)    do_test_keygen(generate_key_between_base10, _a, _b, _res, ++ncount, &nsuccess, &nfailure)
#define TEST_KEY62(_a, _b, _res)    do_test_keygen(generate_key_between_base62, _a, _b, _res, ++ncount, &nsuccess, &nfailure)
#define TEST_KEY95(_a, _b, _res)    do_test_keygen(generate_key_between_base95, _a, _b, _res, ++ncount, &nsuccess, &nfailure)

#define TEST_NKEYS10(_a, _b, _n, _res) do_test_nkeygen(generate_n_keys_between_base10, _a, _b, _n, _res, ++ncount, &nsuccess, &nfailure)
#define TEST_NKEYS62(_a, _b, _n, _res) do_test_nkeygen(generate_n_keys_between_base62, _a, _b, _n, _res, ++ncount, &nsuccess, &nfailure)
#define TEST_NKEYS95(_a, _b, _n, _res) do_test_nkeygen(generate_n_keys_between_base95, _a, _b, _n, _res, ++ncount, &nsuccess, &nfailure)

#define TEST_VALID(_fn, _key, _exp) do_test_validate(_fn, _key, _exp, ++ncount, &nsuccess, &nfailure)

// MARK: - Test helpers

static bool check_result (const char *result, const char *expected) {
    if (result == NULL && expected == NULL) return true;
    if (result == NULL || expected == NULL) return false;
    return strcmp(result, expected) == 0;
}

static void do_test_midpoint (midpoint_callback midpointcb, const char *a, const char *b, const char *expected, int counter, int *nsuccess, int *nfailure) {
    char *result = midpointcb(a, b, NULL);
    bool pass = check_result(result, expected);
    if (pass) (*nsuccess)++; else (*nfailure)++;

    const char *pa = (strlen(a) == 0) ? "[" : a;
    const char *pb = b ? b : "]";
    printf("Test %03d: %8s\t%8s\t%8s\t%s\n", counter, pa, pb,
           result ? result : "NULL", pass ? "PASS" : "ERROR");
    if (result) free(result);
}

static void do_test_incdec (incdec_callback cb, const char *a, const char *expected, int counter, int *nsuccess, int *nfailure) {
    char *result = cb(a);
    bool pass = check_result(result, expected);
    if (pass) (*nsuccess)++; else (*nfailure)++;

    printf("Test %03d: %8s\t%8s\t%8s\t%s\n", counter, a,
           expected ? expected : "NULL", result ? result : "NULL", pass ? "PASS" : "ERROR");
    if (result) free(result);
}

static void do_test_keygen (keygen_callback cb, const char *a, const char *b, const char *expected, int counter, int *nsuccess, int *nfailure) {
    char *result = cb(a, b);
    bool pass = check_result(result, expected);
    if (pass) (*nsuccess)++; else (*nfailure)++;

    printf("Test %03d: %s\t%s\t=> %s\texp %s\t%s\n", counter,
           a ? a : "NULL", b ? b : "NULL",
           result ? result : "NULL", expected ? expected : "NULL",
           pass ? "PASS" : "ERROR");
    if (result) free(result);
}

static void do_test_nkeygen (nkeygen_callback cb, const char *a, const char *b, int n, const char *expected, int counter, int *nsuccess, int *nfailure) {
    char **results = cb(a, b, n);

    char result_buf[2048] = {0};
    if (results) {
        int offset = 0;
        for (int i = 0; i < n; i++) {
            if (i > 0) result_buf[offset++] = ' ';
            int len = (int)strlen(results[i]);
            memcpy(result_buf + offset, results[i], len);
            offset += len;
        }
        result_buf[offset] = '\0';
    }

    const char *pr = results ? result_buf : "NULL";
    bool pass;
    if (results == NULL && expected == NULL) pass = true;
    else if (results == NULL || expected == NULL) pass = false;
    else pass = (strcmp(result_buf, expected) == 0);

    if (pass) (*nsuccess)++; else (*nfailure)++;

    printf("Test %03d: n=%d %s\t%s\t=> %s\texp %s\t%s\n", counter, n,
           a ? a : "NULL", b ? b : "NULL",
           pr, expected ? expected : "NULL", pass ? "PASS" : "ERROR");
    free_keys(results, n);
}

static void do_test_validate (bool (*fn)(const char *), const char *key, bool expected, int counter, int *nsuccess, int *nfailure) {
    bool result = fn(key);
    bool pass = (result == expected);
    if (pass) (*nsuccess)++; else (*nfailure)++;

    printf("Test %03d: \"%s\"\t=> %s\texp %s\t%s\n", counter,
           key ? key : "NULL",
           result ? "true" : "false", expected ? "true" : "false",
           pass ? "PASS" : "ERROR");
}

static void do_test_error (const char *a, const char *b, const char *digits, const char *expected_err, int counter, int *nsuccess, int *nfailure) {
    char *err = NULL;
    char *result = generate_key_between_custom(a, b, digits, &err);
    bool pass;
    if (expected_err == NULL) {
        pass = (result != NULL && err == NULL);
    } else {
        pass = (result == NULL && err != NULL && strcmp(err, expected_err) == 0);
    }
    if (pass) (*nsuccess)++; else (*nfailure)++;

    printf("Test %03d: %s\t%s\t=> r=%s err=%s\t%s\n", counter,
           a ? a : "NULL", b ? b : "NULL",
           result ? result : "NULL", err ? err : "NULL",
           pass ? "PASS" : "ERROR");
    if (result) free(result);
}

#define TEST_ERR(_a, _b, _digits, _err) do_test_error(_a, _b, _digits, _err, ++ncount, &nsuccess, &nfailure)

// MARK: - Midpoint tests

static void test_midpoint_base10 (void) {
    printf("Testing midpoint base10...\n\n");
    START_TEST();

    TEST_BASE10("", NULL, "5");
    TEST_BASE10("5", NULL, "8");
    TEST_BASE10("8", NULL, "9");
    TEST_BASE10("9", NULL, "95");
    TEST_BASE10("95", NULL, "98");
    TEST_BASE10("98", NULL, "99");
    TEST_BASE10("99", NULL, "995");
    TEST_BASE10("1", "2", "15");
    TEST_BASE10("2", "1", NULL);
    TEST_BASE10("", "", NULL);
    TEST_BASE10("0", "1", NULL);
    TEST_BASE10("1", "10", NULL);
    TEST_BASE10("11", "1", NULL);
    TEST_BASE10("001", "001002", "001001");
    TEST_BASE10("001", "001001", "0010005");
    TEST_BASE10("", "5", "3");
    TEST_BASE10("", "3", "2");
    TEST_BASE10("", "2", "1");
    TEST_BASE10("", "1", "05");
    TEST_BASE10("05", "1", "08");
    TEST_BASE10("", "05", "03");
    TEST_BASE10("", "03", "02");
    TEST_BASE10("", "02", "01");
    TEST_BASE10("", "01", "005");
    TEST_BASE10("499", "5", "4995");

    END_TEST();
}

static void test_midpoint_base62 (void) {
    printf("Testing midpoint base62...\n\n");
    START_TEST();

    TEST_BASE62("", NULL, "V");
    TEST_BASE62("V", NULL, "l");
    TEST_BASE62("l", NULL, "t");
    TEST_BASE62("t", NULL, "x");
    TEST_BASE62("x", NULL, "z");
    TEST_BASE62("z", NULL, "zV");
    TEST_BASE62("zV", NULL, "zl");
    TEST_BASE62("zl", NULL, "zt");
    TEST_BASE62("zt", NULL, "zx");
    TEST_BASE62("zx", NULL, "zz");
    TEST_BASE62("zz", NULL, "zzV");
    TEST_BASE62("1", "2", "1V");
    TEST_BASE62("2", "1", NULL);
    TEST_BASE62("", "", NULL);
    TEST_BASE62("0", "1", NULL);
    TEST_BASE62("1", "10", NULL);
    TEST_BASE62("11", "1", NULL);
    TEST_BASE62("001", "001002", "001001");
    TEST_BASE62("001", "001001", "001000V");
    TEST_BASE62("", "V", "G");
    TEST_BASE62("", "G", "8");
    TEST_BASE62("", "8", "4");
    TEST_BASE62("", "4", "2");
    TEST_BASE62("", "2", "1");
    TEST_BASE62("", "1", "0V");
    TEST_BASE62("0V", "1", "0l");
    TEST_BASE62("", "0G", "08");
    TEST_BASE62("", "08", "04");
    TEST_BASE62("", "02", "01");
    TEST_BASE62("", "01", "00V");
    TEST_BASE62("4zz", "5", "4zzV");

    END_TEST();
}

static void test_midpoint_base95 (void) {
    printf("Testing midpoint base95...\n\n");
    START_TEST();

    TEST_BASE95("", NULL, "P");
    TEST_BASE95("P", NULL, "h");
    TEST_BASE95("h", NULL, "t");
    TEST_BASE95("t", NULL, "z");
    TEST_BASE95("z", NULL, "}");
    TEST_BASE95("", "P", "8");
    TEST_BASE95("1", "2", "1P");
    TEST_BASE95("001", "001002", "001(");
    TEST_BASE95("001", "001001", "001(");
    TEST_BASE95("", "8", ",");
    TEST_BASE95("", ",", "&");
    TEST_BASE95("", "&", "#");
    TEST_BASE95("", "1", ")");
    TEST_BASE95("0P", "1", "0h");
    TEST_BASE95("", "0P", "(");
    TEST_BASE95("4~~", "5", "4~~P");

    // Error cases
    TEST_BASE95("2", "1", NULL);    // a >= b
    TEST_BASE95("", "", NULL);      // a >= b (both empty)

    END_TEST();
}

// MARK: - Increment / Decrement tests

static void test_incdec_base10 (void) {
    printf("Testing increment/decrement base10...\n\n");
    START_TEST();

    // Increment base10
    TEST_INC10("a0", "a1");
    TEST_INC10("a1", "a2");
    TEST_INC10("a5", "a6");
    TEST_INC10("a9", "b00");
    TEST_INC10("b09", "b10");
    TEST_INC10("b99", "c000");
    TEST_INC10("Z0", "Z1");
    TEST_INC10("Z9", "a0");
    TEST_INC10("Y09", "Y10");
    TEST_INC10("Y99", "Z0");

    // Decrement base10
    TEST_DEC10("a1", "a0");
    TEST_DEC10("a2", "a1");
    TEST_DEC10("a0", "Z9");
    TEST_DEC10("b00", "a9");
    TEST_DEC10("b10", "b09");
    TEST_DEC10("c000", "b99");
    TEST_DEC10("Z1", "Z0");
    TEST_DEC10("Z0", "Y99");

    // Overflow
    TEST_INC10("z999999999999999999999999999", NULL);
    TEST_DEC10("A00000000000000000000000000", NULL);

    END_TEST();
}

static void test_incdec_base62 (void) {
    printf("Testing increment/decrement base62...\n\n");
    START_TEST();

    // Increment base62
    TEST_INC62("a0", "a1");
    TEST_INC62("az", "b00");
    TEST_INC62("b0z", "b10");
    TEST_INC62("b1z", "b20");
    TEST_INC62("bzz", "c000");
    TEST_INC62("Zy", "Zz");
    TEST_INC62("Zz", "a0");
    TEST_INC62("Yzy", "Yzz");
    TEST_INC62("Yzz", "Z0");
    TEST_INC62("Xyzz", "Xz00");
    TEST_INC62("Xz00", "Xz01");
    TEST_INC62("Xzzz", "Y00");
    TEST_INC62("dABzz", "dAC00");

    // Decrement base62
    TEST_DEC62("a1", "a0");
    TEST_DEC62("b00", "az");
    TEST_DEC62("b10", "b0z");
    TEST_DEC62("b20", "b1z");
    TEST_DEC62("c000", "bzz");
    TEST_DEC62("Zz", "Zy");
    TEST_DEC62("a0", "Zz");
    TEST_DEC62("Yzz", "Yzy");
    TEST_DEC62("Z0", "Yzz");
    TEST_DEC62("Xz00", "Xyzz");
    TEST_DEC62("Xz01", "Xz00");
    TEST_DEC62("Y00", "Xzzz");
    TEST_DEC62("dAC00", "dABzz");

    // Overflow
    TEST_INC62("zzzzzzzzzzzzzzzzzzzzzzzzzzz", NULL);
    TEST_DEC62("A00000000000000000000000000", NULL);

    END_TEST();
}

static void test_incdec_base95 (void) {
    printf("Testing increment/decrement base95...\n\n");
    START_TEST();

    // Increment base95
    TEST_INC95("a ", "a!");
    TEST_INC95("a~", "b  ");
    TEST_INC95("Z~", "a ");
    TEST_INC95("Z ", "Z!");
    TEST_INC95("b !",  "b \"");
    TEST_INC95("b~~", "c   ");

    // Decrement base95
    TEST_DEC95("a!", "a ");
    TEST_DEC95("a ", "Z~");
    TEST_DEC95("b  ", "a~");
    TEST_DEC95("Z!", "Z ");
    TEST_DEC95("c   ", "b~~");

    // Overflow
    TEST_INC95("z~~~~~~~~~~~~~~~~~~~~~~~~~~~", NULL);
    TEST_DEC95("A                          ", NULL);

    END_TEST();
}

// MARK: - Validation tests

static void test_validate_base10 (void) {
    printf("Testing validate base10...\n\n");
    START_TEST();

    // Valid keys
    TEST_VALID(validate_order_key_base10, "a0", true);
    TEST_VALID(validate_order_key_base10, "a1", true);
    TEST_VALID(validate_order_key_base10, "a05", true);
    TEST_VALID(validate_order_key_base10, "b00", true);
    TEST_VALID(validate_order_key_base10, "Zz", true);
    TEST_VALID(validate_order_key_base10, "Z0", true);
    TEST_VALID(validate_order_key_base10, "a01", true);
    TEST_VALID(validate_order_key_base10, "a19", true);

    // Invalid: trailing zero in fractional part
    TEST_VALID(validate_order_key_base10, "a00", false);
    TEST_VALID(validate_order_key_base10, "a10", false);
    TEST_VALID(validate_order_key_base10, "b000", false);

    // Invalid: smallest integer
    TEST_VALID(validate_order_key_base10, "A00000000000000000000000000", false);

    // Invalid: bad head char
    TEST_VALID(validate_order_key_base10, "0", false);
    TEST_VALID(validate_order_key_base10, "9", false);
    TEST_VALID(validate_order_key_base10, "!", false);

    // Invalid: too short for head
    TEST_VALID(validate_order_key_base10, "b", false);
    TEST_VALID(validate_order_key_base10, "b0", false);

    // Invalid: NULL / empty
    TEST_VALID(validate_order_key_base10, NULL, false);
    TEST_VALID(validate_order_key_base10, "", false);

    END_TEST();
}

static void test_validate_base62 (void) {
    printf("Testing validate base62...\n\n");
    START_TEST();

    TEST_VALID(validate_order_key_base62, "a0", true);
    TEST_VALID(validate_order_key_base62, "a1", true);
    TEST_VALID(validate_order_key_base62, "a0V", true);
    TEST_VALID(validate_order_key_base62, "Zz", true);
    TEST_VALID(validate_order_key_base62, "b125", true);
    TEST_VALID(validate_order_key_base62, "zzzzzzzzzzzzzzzzzzzzzzzzzzz", true);
    TEST_VALID(validate_order_key_base62, "A000000000000000000000000001", true);

    // Invalid: trailing zero
    TEST_VALID(validate_order_key_base62, "a00", false);
    TEST_VALID(validate_order_key_base62, "b1230", false);

    // Invalid: smallest integer
    TEST_VALID(validate_order_key_base62, "A00000000000000000000000000", false);

    // Invalid: bad head / too short
    TEST_VALID(validate_order_key_base62, "0", false);
    TEST_VALID(validate_order_key_base62, "b", false);
    TEST_VALID(validate_order_key_base62, NULL, false);
    TEST_VALID(validate_order_key_base62, "", false);

    END_TEST();
}

static void test_validate_base95 (void) {
    printf("Testing validate base95...\n\n");
    START_TEST();

    TEST_VALID(validate_order_key_base95, "a ", true);
    TEST_VALID(validate_order_key_base95, "a!", true);
    TEST_VALID(validate_order_key_base95, "Z~", true);
    TEST_VALID(validate_order_key_base95, "a0", true);
    TEST_VALID(validate_order_key_base95, "a00", true);
    TEST_VALID(validate_order_key_base95, "a01", true);

    // Invalid: trailing space (space is zero digit for base95)
    TEST_VALID(validate_order_key_base95, "a0 ", false);
    TEST_VALID(validate_order_key_base95, "b   ", false);

    // Invalid: smallest integer
    TEST_VALID(validate_order_key_base95, "A                          ", false);

    TEST_VALID(validate_order_key_base95, NULL, false);
    TEST_VALID(validate_order_key_base95, "", false);

    END_TEST();
}

// MARK: - Generate key between tests

static void test_generate_key_between_base10 (void) {
    printf("Testing generate_key_between base10...\n\n");
    START_TEST();

    // Both null
    TEST_KEY10(NULL, NULL, "a0");

    // a is null
    TEST_KEY10(NULL, "a0", "Z9");
    TEST_KEY10(NULL, "Z9", "Z8");
    TEST_KEY10(NULL, "a05", "a0");
    TEST_KEY10(NULL, "b999", "b99");

    // b is null
    TEST_KEY10("a0", NULL, "a1");
    TEST_KEY10("a1", NULL, "a2");
    TEST_KEY10("a9", NULL, "b00");
    TEST_KEY10("b09", NULL, "b10");
    TEST_KEY10("b99", NULL, "c000");

    // Both non-null
    TEST_KEY10("a0", "a1", "a05");
    TEST_KEY10("a0", "a2", "a1");
    TEST_KEY10("a1", "a2", "a15");
    TEST_KEY10("a5", "a6", "a55");
    TEST_KEY10("b125", "b129", "b127");

    // Same integer part, deeper midpoints
    TEST_KEY10("a0", "a05", "a03");
    TEST_KEY10("a0", "a03", "a02");
    TEST_KEY10("a0", "a02", "a01");

    // Different integer parts, consecutive
    TEST_KEY10("Z9", "a0", "Z95");
    TEST_KEY10("Z9", "a1", "a0");

    // Invalid keys
    TEST_KEY10("a00", NULL, NULL);
    TEST_KEY10("a00", "a1", NULL);
    TEST_KEY10("a1", "a0", NULL);
    TEST_KEY10(NULL, "A00000000000000000000000000", NULL);

    // Smallest integer edge case
    TEST_KEY10(NULL, "A000000000000000000000000001", "A0000000000000000000000000005");

    // Largest integer edge case
    TEST_KEY10("z999999999999999999999999999", NULL, "z9999999999999999999999999995");

    END_TEST();
}

static void test_generate_key_between_base62 (void) {
    printf("Testing generate_key_between base62...\n\n");
    START_TEST();

    // Both null
    TEST_KEY62(NULL, NULL, "a0");

    // a is null
    TEST_KEY62(NULL, "a0", "Zz");
    TEST_KEY62(NULL, "Zz", "Zy");
    TEST_KEY62(NULL, "Y00", "Xzzz");
    TEST_KEY62(NULL, "a0V", "a0");
    TEST_KEY62(NULL, "b999", "b99");

    // b is null
    TEST_KEY62("a0", NULL, "a1");
    TEST_KEY62("a1", NULL, "a2");
    TEST_KEY62("bzz", NULL, "c000");
    TEST_KEY62("zzzzzzzzzzzzzzzzzzzzzzzzzzy", NULL, "zzzzzzzzzzzzzzzzzzzzzzzzzzz");
    TEST_KEY62("zzzzzzzzzzzzzzzzzzzzzzzzzzz", NULL, "zzzzzzzzzzzzzzzzzzzzzzzzzzzV");

    // Both non-null
    TEST_KEY62("a0", "a1", "a0V");
    TEST_KEY62("a1", "a2", "a1V");
    TEST_KEY62("a0V", "a1", "a0l");
    TEST_KEY62("Zz", "a0", "ZzV");
    TEST_KEY62("Zz", "a1", "a0");
    TEST_KEY62("a0", "a0V", "a0G");
    TEST_KEY62("a0", "a0G", "a08");
    TEST_KEY62("b125", "b129", "b127");
    TEST_KEY62("a0", "a1V", "a1");
    TEST_KEY62("Zz", "a01", "a0");

    // Invalid keys
    TEST_KEY62(NULL, "A00000000000000000000000000", NULL);
    TEST_KEY62("a00", NULL, NULL);
    TEST_KEY62("a00", "a1", NULL);
    TEST_KEY62("a1", "a0", NULL);

    // Smallest integer edge case
    TEST_KEY62(NULL, "A000000000000000000000000001", "A000000000000000000000000000V");

    // Largest integer, can't increment -> use midpoint
    TEST_KEY62("zzzzzzzzzzzzzzzzzzzzzzzzzzz", NULL, "zzzzzzzzzzzzzzzzzzzzzzzzzzzV");

    END_TEST();
}

static void test_generate_key_between_base95 (void) {
    printf("Testing generate_key_between base95...\n\n");
    START_TEST();

    TEST_KEY95(NULL, NULL, "a ");
    TEST_KEY95("a ", NULL, "a!");
    TEST_KEY95(NULL, "a ", "Z~");
    TEST_KEY95("a00", "a01", "a00P");
    TEST_KEY95("a0/", "a00", "a0/P");
    TEST_KEY95("a~", NULL, "b  ");
    TEST_KEY95("Z~", NULL, "a ");
    TEST_KEY95("a0", "a0V", "a0;");
    TEST_KEY95("a  1", "a  2", "a  1P");

    // Invalid keys
    TEST_KEY95("a0 ", "a0!", NULL);
    TEST_KEY95("b   ", NULL, NULL);
    TEST_KEY95(NULL, "A                          ", NULL);

    // Smallest integer edge case
    TEST_KEY95(NULL, "A                          0", "A                          (");

    // Additional base95 cases
    TEST_KEY95("a!", NULL, "a\"");
    TEST_KEY95(NULL, "Z~", "Z}");
    TEST_KEY95("a ", "a!", "a P");

    END_TEST();
}

// MARK: - Generate N keys between tests

static void test_nkeys_base10 (void) {
    printf("Testing generate_n_keys_between base10...\n\n");
    START_TEST();

    TEST_NKEYS10(NULL, NULL, 5, "a0 a1 a2 a3 a4");
    TEST_NKEYS10("a4", NULL, 10, "a5 a6 a7 a8 a9 b00 b01 b02 b03 b04");
    TEST_NKEYS10(NULL, "a0", 5, "Z5 Z6 Z7 Z8 Z9");
    TEST_NKEYS10("a0", "a2", 20, "a01 a02 a03 a035 a04 a05 a06 a07 a08 a09 a1 a11 a12 a13 a14 a15 a16 a17 a18 a19");

    // Edge cases
    TEST_NKEYS10(NULL, NULL, 1, "a0");
    TEST_NKEYS10("a0", "a1", 1, "a05");
    TEST_NKEYS10("a0", "a1", 2, "a03 a05");

    END_TEST();
}

static void test_nkeys_base62 (void) {
    printf("Testing generate_n_keys_between base62...\n\n");
    START_TEST();

    TEST_NKEYS62(NULL, NULL, 5, "a0 a1 a2 a3 a4");
    TEST_NKEYS62("a4", NULL, 10, "a5 a6 a7 a8 a9 aA aB aC aD aE");
    TEST_NKEYS62(NULL, "a0", 5, "Zv Zw Zx Zy Zz");
    TEST_NKEYS62("a0", "a2", 5, "a0G a0V a1 a1G a1V");

    // Edge cases
    TEST_NKEYS62(NULL, NULL, 1, "a0");
    TEST_NKEYS62("a0", "a1", 1, "a0V");
    TEST_NKEYS62("a0", "a1", 2, "a0G a0V");

    END_TEST();
}

static void test_nkeys_base95 (void) {
    printf("Testing generate_n_keys_between base95...\n\n");
    START_TEST();

    TEST_NKEYS95(NULL, NULL, 5, "a  a! a\" a# a$");
    TEST_NKEYS95(NULL, "a ", 5, "Zz Z{ Z| Z} Z~");
    TEST_NKEYS95("a ", NULL, 5, "a! a\" a# a$ a%");
    TEST_NKEYS95("a ", "a\"", 3, "a P a! a!P");

    // Edge cases
    TEST_NKEYS95(NULL, NULL, 1, "a ");
    TEST_NKEYS95("a ", "a!", 1, "a P");

    END_TEST();
}

// MARK: - Custom base tests

static void test_custom_base (void) {
    printf("Testing custom base API...\n\n");
    START_TEST();

    // Custom base10 should match base10 functions
    {
        const char *d = "0123456789";
        char *err = NULL;

        char *r = generate_key_between_custom(NULL, NULL, d, &err);
        bool pass = (r && strcmp(r, "a0") == 0 && err == NULL);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: custom10(NULL,NULL) => %s\t%s\n", ++ncount, r ? r : "NULL", pass ? "PASS" : "ERROR");
        free(r);

        r = generate_key_between_custom("a0", NULL, d, &err);
        pass = (r && strcmp(r, "a1") == 0);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: custom10(a0,NULL) => %s\t%s\n", ++ncount, r ? r : "NULL", pass ? "PASS" : "ERROR");
        free(r);

        r = generate_key_between_custom("a0", "a1", d, &err);
        pass = (r && strcmp(r, "a05") == 0);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: custom10(a0,a1) => %s\t%s\n", ++ncount, r ? r : "NULL", pass ? "PASS" : "ERROR");
        free(r);
    }

    // Custom base62 should match base62 functions
    {
        const char *d = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        char *err = NULL;

        char *r = generate_key_between_custom(NULL, NULL, d, &err);
        bool pass = (r && strcmp(r, "a0") == 0);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: custom62(NULL,NULL) => %s\t%s\n", ++ncount, r ? r : "NULL", pass ? "PASS" : "ERROR");
        free(r);

        r = generate_key_between_custom("a0", "a1", d, &err);
        pass = (r && strcmp(r, "a0V") == 0);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: custom62(a0,a1) => %s\t%s\n", ++ncount, r ? r : "NULL", pass ? "PASS" : "ERROR");
        free(r);
    }

    // Custom validate
    {
        const char *d = "0123456789";
        bool pass;

        pass = (validate_order_key_custom("a0", d) == true);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: validate_custom(a0) => %s\t%s\n", ++ncount, "true", pass ? "PASS" : "ERROR");

        pass = (validate_order_key_custom("a00", d) == false);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: validate_custom(a00) => %s\t%s\n", ++ncount, "false", pass ? "PASS" : "ERROR");

        pass = (validate_order_key_custom(NULL, d) == false);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: validate_custom(NULL) => %s\t%s\n", ++ncount, "false", pass ? "PASS" : "ERROR");
    }

    // Custom generate_n_keys_between
    {
        const char *d = "0123456789";
        char **r = generate_n_keys_between_custom(NULL, NULL, 3, d, NULL);
        char buf[256] = {0};
        if (r) {
            int off = 0;
            for (int i = 0; i < 3; i++) {
                if (i) buf[off++] = ' ';
                int l = (int)strlen(r[i]);
                memcpy(buf + off, r[i], l);
                off += l;
            }
        }
        bool pass = (r && strcmp(buf, "a0 a1 a2") == 0);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: custom_n(NULL,NULL,3) => %s\t%s\n", ++ncount, r ? buf : "NULL", pass ? "PASS" : "ERROR");
        free_keys(r, 3);
    }

    END_TEST();
}

// MARK: - Error reporting tests

static void test_error_reporting (void) {
    printf("Testing error reporting...\n\n");
    START_TEST();

    const char *d10 = "0123456789";
    const char *d62 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    // Invalid order key (trailing zero)
    TEST_ERR("a00", NULL, d10, "invalid order key");
    TEST_ERR(NULL, "a00", d10, "invalid order key");
    TEST_ERR("a00", "a1", d10, "invalid order key");

    // Invalid order key (smallest integer)
    TEST_ERR(NULL, "A00000000000000000000000000", d62, "invalid order key");

    // Invalid order key (bad head)
    TEST_ERR("0", NULL, d10, "invalid order key");

    // Invalid order key (too short)
    TEST_ERR("b", NULL, d10, "invalid order key");
    TEST_ERR("b0", NULL, d10, "invalid order key");

    // a >= b
    TEST_ERR("a1", "a0", d10, "a >= b");
    TEST_ERR("a1", "a1", d10, "a >= b");
    TEST_ERR("b00", "a5", d10, "a >= b");

    // No error cases: err should stay NULL
    TEST_ERR(NULL, NULL, d10, NULL);
    TEST_ERR("a0", NULL, d10, NULL);
    TEST_ERR(NULL, "a0", d10, NULL);
    TEST_ERR("a0", "a1", d10, NULL);

    END_TEST();
}

// MARK: - Ordering invariant test

static void test_ordering_invariant (void) {
    printf("Testing ordering invariant...\n\n");
    START_TEST();

    // Generate 100 keys in sequence (b=NULL) for each base, verify lexicographic order
    {
        char **keys = generate_n_keys_between_base62(NULL, NULL, 100);
        bool pass = (keys != NULL);
        if (pass) {
            for (int i = 1; i < 100; i++) {
                if (strcmp(keys[i-1], keys[i]) >= 0) { pass = false; break; }
            }
        }
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: 100 sequential base62 keys are strictly ordered\t%s\n", ++ncount, pass ? "PASS" : "ERROR");
        free_keys(keys, 100);
    }

    {
        char **keys = generate_n_keys_between_base10(NULL, NULL, 100);
        bool pass = (keys != NULL);
        if (pass) {
            for (int i = 1; i < 100; i++) {
                if (strcmp(keys[i-1], keys[i]) >= 0) { pass = false; break; }
            }
        }
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: 100 sequential base10 keys are strictly ordered\t%s\n", ++ncount, pass ? "PASS" : "ERROR");
        free_keys(keys, 100);
    }

    {
        char **keys = generate_n_keys_between_base95(NULL, NULL, 100);
        bool pass = (keys != NULL);
        if (pass) {
            for (int i = 1; i < 100; i++) {
                if (strcmp(keys[i-1], keys[i]) >= 0) { pass = false; break; }
            }
        }
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: 100 sequential base95 keys are strictly ordered\t%s\n", ++ncount, pass ? "PASS" : "ERROR");
        free_keys(keys, 100);
    }

    // 100000-key stress test for each base
    {
        char **keys = generate_n_keys_between_base62(NULL, NULL, 100000);
        bool pass = (keys != NULL);
        if (pass) {
            for (int i = 1; i < 100000; i++) {
                if (strcmp(keys[i-1], keys[i]) >= 0) { pass = false; break; }
            }
        }
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: 100000 sequential base62 keys are strictly ordered\t%s\n", ++ncount, pass ? "PASS" : "ERROR");
        free_keys(keys, 100000);
    }

    {
        char **keys = generate_n_keys_between_base10(NULL, NULL, 100000);
        bool pass = (keys != NULL);
        if (pass) {
            for (int i = 1; i < 100000; i++) {
                if (strcmp(keys[i-1], keys[i]) >= 0) { pass = false; break; }
            }
        }
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: 100000 sequential base10 keys are strictly ordered\t%s\n", ++ncount, pass ? "PASS" : "ERROR");
        free_keys(keys, 100000);
    }

    {
        char **keys = generate_n_keys_between_base95(NULL, NULL, 100000);
        bool pass = (keys != NULL);
        if (pass) {
            for (int i = 1; i < 100000; i++) {
                if (strcmp(keys[i-1], keys[i]) >= 0) { pass = false; break; }
            }
        }
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: 100000 sequential base95 keys are strictly ordered\t%s\n", ++ncount, pass ? "PASS" : "ERROR");
        free_keys(keys, 100000);
    }

    // Generate keys between two existing keys, verify they interleave correctly
    {
        char **keys = generate_n_keys_between_base62("a0", "a2", 50);
        bool pass = (keys != NULL);
        if (pass) {
            // All keys must be > "a0" and < "a2"
            for (int i = 0; i < 50; i++) {
                if (strcmp(keys[i], "a0") <= 0 || strcmp(keys[i], "a2") >= 0) { pass = false; break; }
            }
            // Keys must be strictly ordered
            if (pass) {
                for (int i = 1; i < 50; i++) {
                    if (strcmp(keys[i-1], keys[i]) >= 0) { pass = false; break; }
                }
            }
        }
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: 50 base62 keys between a0..a2 are bounded and ordered\t%s\n", ++ncount, pass ? "PASS" : "ERROR");
        free_keys(keys, 50);
    }

    // Generate keys with a=NULL, verify ordering and < b
    {
        char **keys = generate_n_keys_between_base62(NULL, "a0", 50);
        bool pass = (keys != NULL);
        if (pass) {
            for (int i = 0; i < 50; i++) {
                if (strcmp(keys[i], "a0") >= 0) { pass = false; break; }
            }
            if (pass) {
                for (int i = 1; i < 50; i++) {
                    if (strcmp(keys[i-1], keys[i]) >= 0) { pass = false; break; }
                }
            }
        }
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: 50 base62 keys before a0 are bounded and ordered\t%s\n", ++ncount, pass ? "PASS" : "ERROR");
        free_keys(keys, 50);
    }

    // Insert between adjacent generated keys to verify it always works
    {
        char *k1 = generate_key_between_base62(NULL, NULL);
        char *k2 = generate_key_between_base62(k1, NULL);
        char *mid = generate_key_between_base62(k1, k2);
        bool pass = (k1 && k2 && mid && strcmp(k1, mid) < 0 && strcmp(mid, k2) < 0);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: insert between two generated keys\t%s\n", ++ncount, pass ? "PASS" : "ERROR");
        free(k1); free(k2); free(mid);
    }

    // All generated keys must validate
    {
        char **keys = generate_n_keys_between_base62(NULL, NULL, 20);
        bool pass = (keys != NULL);
        if (pass) {
            for (int i = 0; i < 20; i++) {
                if (!validate_order_key_base62(keys[i])) { pass = false; break; }
            }
        }
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: all 20 generated base62 keys pass validation\t%s\n", ++ncount, pass ? "PASS" : "ERROR");
        free_keys(keys, 20);
    }

    {
        char **keys = generate_n_keys_between_base10(NULL, NULL, 20);
        bool pass = (keys != NULL);
        if (pass) {
            for (int i = 0; i < 20; i++) {
                if (!validate_order_key_base10(keys[i])) { pass = false; break; }
            }
        }
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: all 20 generated base10 keys pass validation\t%s\n", ++ncount, pass ? "PASS" : "ERROR");
        free_keys(keys, 20);
    }

    {
        char **keys = generate_n_keys_between_base95(NULL, NULL, 20);
        bool pass = (keys != NULL);
        if (pass) {
            for (int i = 0; i < 20; i++) {
                if (!validate_order_key_base95(keys[i])) { pass = false; break; }
            }
        }
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: all 20 generated base95 keys pass validation\t%s\n", ++ncount, pass ? "PASS" : "ERROR");
        free_keys(keys, 20);
    }

    END_TEST();
}

// MARK: - N keys edge cases

static void test_nkeys_edge_cases (void) {
    printf("Testing generate_n_keys_between edge cases...\n\n");
    START_TEST();

    // n=0 returns NULL
    {
        char **r = generate_n_keys_between_base62(NULL, NULL, 0);
        bool pass = (r == NULL);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: nkeys(NULL,NULL,0) => NULL\t%s\n", ++ncount, pass ? "PASS" : "ERROR");
    }

    {
        char **r = generate_n_keys_between_base10("a0", "a1", 0);
        bool pass = (r == NULL);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: nkeys10(a0,a1,0) => NULL\t%s\n", ++ncount, pass ? "PASS" : "ERROR");
    }

    // Error propagation: invalid key should return NULL
    {
        char **r = generate_n_keys_between_base62("a00", NULL, 5);
        bool pass = (r == NULL);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: nkeys62(invalid a00,NULL,5) => NULL\t%s\n", ++ncount, pass ? "PASS" : "ERROR");
    }

    {
        char **r = generate_n_keys_between_base10(NULL, "a00", 3);
        bool pass = (r == NULL);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: nkeys10(NULL,invalid a00,3) => NULL\t%s\n", ++ncount, pass ? "PASS" : "ERROR");
    }

    {
        char **r = generate_n_keys_between_base62("a1", "a0", 2);
        bool pass = (r == NULL);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: nkeys62(a1,a0,2) a>=b => NULL\t%s\n", ++ncount, pass ? "PASS" : "ERROR");
    }

    // Custom n_keys error propagation
    {
        char *err = NULL;
        char **r = generate_n_keys_between_custom("a00", NULL, 3, "0123456789", &err);
        bool pass = (r == NULL && err != NULL && strcmp(err, "invalid order key") == 0);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: custom_n(invalid,NULL,3) err=%s\t%s\n", ++ncount, err ? err : "NULL", pass ? "PASS" : "ERROR");
    }

    {
        char *err = NULL;
        char **r = generate_n_keys_between_custom("a1", "a0", 2, "0123456789", &err);
        bool pass = (r == NULL && err != NULL && strcmp(err, "a >= b") == 0);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: custom_n(a1,a0,2) err=%s\t%s\n", ++ncount, err ? err : "NULL", pass ? "PASS" : "ERROR");
    }

    // Large n: 100 keys with b=NULL
    {
        char **keys = generate_n_keys_between_base62("a0", NULL, 100);
        bool pass = (keys != NULL);
        if (pass) {
            for (int i = 1; i < 100; i++) {
                if (strcmp(keys[i-1], keys[i]) >= 0) { pass = false; break; }
            }
        }
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: 100 base62 keys after a0 (b=NULL)\t%s\n", ++ncount, pass ? "PASS" : "ERROR");
        free_keys(keys, 100);
    }

    // Large n: 100 keys with a=NULL
    {
        char **keys = generate_n_keys_between_base62(NULL, "a0", 100);
        bool pass = (keys != NULL);
        if (pass) {
            for (int i = 1; i < 100; i++) {
                if (strcmp(keys[i-1], keys[i]) >= 0) { pass = false; break; }
            }
        }
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: 100 base62 keys before a0 (a=NULL)\t%s\n", ++ncount, pass ? "PASS" : "ERROR");
        free_keys(keys, 100);
    }

    // Large n: 100 keys between two keys (divide-and-conquer)
    {
        char **keys = generate_n_keys_between_base62("a0", "a2", 100);
        bool pass = (keys != NULL);
        if (pass) {
            for (int i = 1; i < 100; i++) {
                if (strcmp(keys[i-1], keys[i]) >= 0) { pass = false; break; }
            }
        }
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: 100 base62 keys between a0..a2 (d&c)\t%s\n", ++ncount, pass ? "PASS" : "ERROR");
        free_keys(keys, 100);
    }

    // Large n: base10
    {
        char **keys = generate_n_keys_between_base10("a0", NULL, 100);
        bool pass = (keys != NULL);
        if (pass) {
            for (int i = 1; i < 100; i++) {
                if (strcmp(keys[i-1], keys[i]) >= 0) { pass = false; break; }
            }
        }
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: 100 base10 keys after a0 (b=NULL)\t%s\n", ++ncount, pass ? "PASS" : "ERROR");
        free_keys(keys, 100);
    }

    // Large n: base95
    {
        char **keys = generate_n_keys_between_base95(NULL, NULL, 100);
        bool pass = (keys != NULL);
        if (pass) {
            for (int i = 1; i < 100; i++) {
                if (strcmp(keys[i-1], keys[i]) >= 0) { pass = false; break; }
            }
        }
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: 100 base95 keys (NULL,NULL)\t%s\n", ++ncount, pass ? "PASS" : "ERROR");
        free_keys(keys, 100);
    }

    END_TEST();
}

// MARK: - Midpoint error parameter test

static void test_midpoint_error_param (void) {
    printf("Testing midpoint error parameter...\n\n");
    START_TEST();

    // base10: error should be set on failure
    {
        char *err = NULL;
        char *r = midpoint_base10("2", "1", &err);
        bool pass = (r == NULL && err != NULL);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: midpoint10(2,1) err=%s\t%s\n", ++ncount, err ? err : "NULL", pass ? "PASS" : "ERROR");
        free(r);
    }

    {
        char *err = NULL;
        char *r = midpoint_base10("", "", &err);
        bool pass = (r == NULL && err != NULL);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: midpoint10('','') err=%s\t%s\n", ++ncount, err ? err : "NULL", pass ? "PASS" : "ERROR");
        free(r);
    }

    // base10: no error on success
    {
        char *err = NULL;
        char *r = midpoint_base10("1", "2", &err);
        bool pass = (r != NULL && err == NULL);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: midpoint10(1,2) ok=%s err=%s\t%s\n", ++ncount, r ? r : "NULL", err ? err : "NULL", pass ? "PASS" : "ERROR");
        free(r);
    }

    // base62
    {
        char *err = NULL;
        char *r = midpoint_base62("2", "1", &err);
        bool pass = (r == NULL && err != NULL);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: midpoint62(2,1) err=%s\t%s\n", ++ncount, err ? err : "NULL", pass ? "PASS" : "ERROR");
        free(r);
    }

    {
        char *err = NULL;
        char *r = midpoint_base62("", NULL, &err);
        bool pass = (r != NULL && err == NULL);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: midpoint62('',NULL) ok=%s\t%s\n", ++ncount, r ? r : "NULL", pass ? "PASS" : "ERROR");
        free(r);
    }

    // base95
    {
        char *err = NULL;
        char *r = midpoint_base95("2", "1", &err);
        bool pass = (r == NULL && err != NULL);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: midpoint95(2,1) err=%s\t%s\n", ++ncount, err ? err : "NULL", pass ? "PASS" : "ERROR");
        free(r);
    }

    {
        char *err = NULL;
        char *r = midpoint_base95("", NULL, &err);
        bool pass = (r != NULL && err == NULL);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: midpoint95('',NULL) ok=%s\t%s\n", ++ncount, r ? r : "NULL", pass ? "PASS" : "ERROR");
        free(r);
    }

    END_TEST();
}

// MARK: - free_keys safety test

static void test_free_keys_null (void) {
    printf("Testing free_keys with NULL...\n\n");
    START_TEST();

    // Should not crash
    free_keys(NULL, 0);
    free_keys(NULL, 5);
    free_keys(NULL, 100);
    nsuccess++;
    printf("Test %03d: free_keys(NULL, ...) did not crash\t%s\n", ++ncount, "PASS");

    END_TEST();
}

// MARK: - Default API tests (base62 wrappers)

static void test_default_api (void) {
    printf("Testing default API (base62 wrappers)...\n\n");
    START_TEST();

    // generate_key_between matches base62
    {
        char *r = generate_key_between(NULL, NULL);
        bool pass = (r && strcmp(r, "a0") == 0);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: generate_key_between(NULL,NULL) => %s\t%s\n", ++ncount, r ? r : "NULL", pass ? "PASS" : "ERROR");
        free(r);
    }

    {
        char *r = generate_key_between("a0", "a1");
        bool pass = (r && strcmp(r, "a0V") == 0);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: generate_key_between(a0,a1) => %s\t%s\n", ++ncount, r ? r : "NULL", pass ? "PASS" : "ERROR");
        free(r);
    }

    {
        char *r = generate_key_between("a0", NULL);
        bool pass = (r && strcmp(r, "a1") == 0);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: generate_key_between(a0,NULL) => %s\t%s\n", ++ncount, r ? r : "NULL", pass ? "PASS" : "ERROR");
        free(r);
    }

    {
        char *r = generate_key_between(NULL, "a0");
        bool pass = (r && strcmp(r, "Zz") == 0);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: generate_key_between(NULL,a0) => %s\t%s\n", ++ncount, r ? r : "NULL", pass ? "PASS" : "ERROR");
        free(r);
    }

    // generate_n_keys_between matches base62
    {
        char **keys = generate_n_keys_between(NULL, NULL, 5);
        char buf[256] = {0};
        if (keys) {
            int off = 0;
            for (int i = 0; i < 5; i++) {
                if (i) buf[off++] = ' ';
                int l = (int)strlen(keys[i]);
                memcpy(buf + off, keys[i], l);
                off += l;
            }
        }
        bool pass = (keys && strcmp(buf, "a0 a1 a2 a3 a4") == 0);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: generate_n_keys_between(NULL,NULL,5) => %s\t%s\n", ++ncount, keys ? buf : "NULL", pass ? "PASS" : "ERROR");
        free_keys(keys, 5);
    }

    // validate_order_key matches base62
    {
        bool pass = (validate_order_key("a0") == true && validate_order_key("a00") == false && validate_order_key(NULL) == false);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: validate_order_key\t%s\n", ++ncount, pass ? "PASS" : "ERROR");
    }

    // midpoint matches base62
    {
        char *err = NULL;
        char *r = midpoint("", NULL, &err);
        bool pass = (r && strcmp(r, "V") == 0 && err == NULL);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: midpoint('',NULL) => %s\t%s\n", ++ncount, r ? r : "NULL", pass ? "PASS" : "ERROR");
        free(r);
    }

    {
        char *err = NULL;
        char *r = midpoint("2", "1", &err);
        bool pass = (r == NULL && err != NULL);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: midpoint(2,1) err=%s\t%s\n", ++ncount, err ? err : "NULL", pass ? "PASS" : "ERROR");
        free(r);
    }

    // increment_integer matches base62
    {
        char *r = increment_integer("a0");
        bool pass = (r && strcmp(r, "a1") == 0);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: increment_integer(a0) => %s\t%s\n", ++ncount, r ? r : "NULL", pass ? "PASS" : "ERROR");
        free(r);
    }

    {
        char *r = increment_integer("az");
        bool pass = (r && strcmp(r, "b00") == 0);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: increment_integer(az) => %s\t%s\n", ++ncount, r ? r : "NULL", pass ? "PASS" : "ERROR");
        free(r);
    }

    // decrement_integer matches base62
    {
        char *r = decrement_integer("a1");
        bool pass = (r && strcmp(r, "a0") == 0);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: decrement_integer(a1) => %s\t%s\n", ++ncount, r ? r : "NULL", pass ? "PASS" : "ERROR");
        free(r);
    }

    {
        char *r = decrement_integer("a0");
        bool pass = (r && strcmp(r, "Zz") == 0);
        if (pass) nsuccess++; else nfailure++;
        printf("Test %03d: decrement_integer(a0) => %s\t%s\n", ++ncount, r ? r : "NULL", pass ? "PASS" : "ERROR");
        free(r);
    }

    END_TEST();
}

// MARK: - Main

int main (int argc, const char * argv[]) {
    // Midpoint
    test_midpoint_base10();
    test_midpoint_base62();
    test_midpoint_base95();

    // Increment / Decrement
    test_incdec_base10();
    test_incdec_base62();
    test_incdec_base95();

    // Validation
    test_validate_base10();
    test_validate_base62();
    test_validate_base95();

    // Generate key between
    test_generate_key_between_base10();
    test_generate_key_between_base62();
    test_generate_key_between_base95();

    // Generate N keys between
    test_nkeys_base10();
    test_nkeys_base62();
    test_nkeys_base95();

    // Custom base
    test_custom_base();

    // Error reporting
    test_error_reporting();

    // Ordering invariant
    test_ordering_invariant();

    // N keys edge cases
    test_nkeys_edge_cases();

    // Midpoint error parameter
    test_midpoint_error_param();

    // free_keys safety
    test_free_keys_null();

    // Default API
    test_default_api();

    // Grand total
    printf("========================================\n");
    printf("GRAND TOTAL: %d tests, %d passed, %d failed\n", total_count, total_success, total_failure);
    printf("========================================\n");

    return total_failure > 0 ? 1 : 0;
}
