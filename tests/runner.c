#include "test.h"

#include <stdio.h>

int g_tests_run;
int g_tests_failed;

int tack_run_tests(const TackTest *tests, int count)
{
    int i;

    g_tests_run = 0;
    g_tests_failed = 0;

    for (i = 0; i < count; i++) {
        printf("TEST %s\n", tests[i].name);
        tests[i].fn();
    }

    printf("\n%d tests, %d failed\n", g_tests_run, g_tests_failed);
    return g_tests_failed == 0 ? 0 : 1;
}
