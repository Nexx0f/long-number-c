#include "long_number.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdarg.h>

int TAB_LEVEL = 0;

void print_tabs()
{
    for (int i = 0; i < TAB_LEVEL; i++)
        printf("  ");
}

char GROUP_FAILED = 0;
int GROUPS_FAILED = 0;

#define verify(condition) \
    if (!(condition)) \
    { \
        print_tabs(); \
        printf("verification failed: '%s' is false\n", #condition); \
        GROUP_FAILED = 1; \
        break; \
    } \
    else \
    { \
        print_tabs(); \
        printf("verification ok: '%s'\n", #condition); \
    }

#define begin_test_group(name) \
    { \
        GROUP_FAILED = 0; \
        TAB_LEVEL++; \
        printf("Test group '%s'\n", name); \
        do \
        {

#define end_test_group() \
        } \
        while (0); \
        TAB_LEVEL--; \
        if (GROUP_FAILED) \
        { \
            printf("test group failed\n"); \
            GROUPS_FAILED++; \
        } \
        else \
        { \
            printf("test group ok\n"); \
        } \
    }

#define check_ok() verify(long_number_errno == ERROR_OK);

void test_utility_functions()
{
    begin_test_group("number utility functions");
    
    // num_is_null
    number a = get_null_num();
    check_ok();
    verify(num_is_null(a));
    check_ok();
    
    a.digits = calloc(10, sizeof(int));
    verify(!num_is_null(a));
    check_ok();
    free(a.digits);
    
    // num_parse & num_free
    
    // positive
    a = num_parse("123");
    check_ok();
    verify(a.n == 3 &&
           !a.is_negative &&
           a.digits[0] == 3 &&
           a.digits[1] == 2 &&
           a.digits[2] == 1);
    
    num_free(a);
    
    // negative
    a = num_parse("-701");
    check_ok();
    verify(a.n == 3 &&
           a.is_negative &&
           a.digits[0] == 1 &&
           a.digits[1] == 0 &&
           a.digits[2] == 7);
    
    num_free(a);
    
    // invalid numbers
    a = num_parse("-+100");
    verify(long_number_errno == ERROR_INVALID_FORMAT);
    a = num_parse("-");
    verify(long_number_errno == ERROR_INVALID_FORMAT);
    a = num_parse("");
    verify(long_number_errno == ERROR_INVALID_FORMAT);
    a = num_parse("100a100");
    verify(long_number_errno == ERROR_INVALID_FORMAT);
    
    end_test_group();
}

void run_unit_tests()
{
    test_utility_functions();
}

int main()
{
    run_unit_tests();
    
    if (GROUPS_FAILED > 0)
        return 1;
    
    return 0;
}
