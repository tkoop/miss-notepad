#ifndef MISSNOTEPAD_TEST_H
#define MISSNOTEPAD_TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int g_tests_run;
extern int g_tests_failed;

#define ASSERT_TRUE(msg, cond)                                                 \
    do {                                                                       \
        g_tests_run++;                                                         \
        if (!(cond)) {                                                         \
            g_tests_failed++;                                                  \
            fprintf(stderr, "  FAIL: %s (%s:%d)\n", (msg), __FILE__,           \
                    __LINE__);                                                 \
        }                                                                      \
    } while (0)

#define ASSERT_EQ_INT(msg, expected, actual)                                   \
    do {                                                                       \
        int _e = (int)(expected);                                              \
        int _a = (int)(actual);                                                \
        g_tests_run++;                                                         \
        if (_e != _a) {                                                        \
            g_tests_failed++;                                                  \
            fprintf(stderr, "  FAIL: %s — expected %d, got %d (%s:%d)\n",      \
                    (msg), _e, _a, __FILE__, __LINE__);                        \
        }                                                                      \
    } while (0)

#define ASSERT_STREQ(msg, expected, actual)                                    \
    do {                                                                       \
        const char *_e = (expected);                                           \
        const char *_a = (actual);                                             \
        g_tests_run++;                                                         \
        if ((_e == NULL && _a != NULL) || (_e != NULL && _a == NULL) ||        \
            (_e != NULL && _a != NULL && strcmp(_e, _a) != 0)) {               \
            g_tests_failed++;                                                  \
            fprintf(stderr, "  FAIL: %s — expected \"%s\", got \"%s\" "        \
                            "(%s:%d)\n",                                       \
                    (msg), _e ? _e : "(null)", _a ? _a : "(null)", __FILE__,   \
                    __LINE__);                                                 \
        }                                                                      \
    } while (0)

#define ASSERT_NULL(msg, ptr) ASSERT_TRUE((msg), (ptr) == NULL)
#define ASSERT_NOT_NULL(msg, ptr) ASSERT_TRUE((msg), (ptr) != NULL)

typedef void (*missnotepad_test_fn)(void);

typedef struct {
    const char *name;
    missnotepad_test_fn fn;
} MissNotepadTest;

int missnotepad_run_tests(const MissNotepadTest *tests, int count);

#endif
