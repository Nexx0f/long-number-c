#include "long_number.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>

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
    
    a = num_free(a);
    num_free(a);
    
    // explicit +
    a = num_parse("+701");
    check_ok();
    verify(a.n == 3 &&
           !a.is_negative &&
           a.digits[0] == 1 &&
           a.digits[1] == 0 &&
           a.digits[2] == 7);
    
    a = num_free(a);
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
    a = num_parse("a100");
    verify(long_number_errno == ERROR_INVALID_FORMAT);
    a = num_parse(0);
    verify(long_number_errno == ERROR_INVALID_ARGUMENT);
    
    end_test_group();
}

char check_num_read(const char* str, num_error_t exp_error, char exp_negative, unsigned exp_n, const char* exp_digits)
{
    FILE* test_in = fopen("tmp.txt", "w");
    assert(test_in);
    fprintf(test_in, "%s", str);
    fclose(test_in);
    
    test_in = fopen("tmp.txt", "r");
    
    number num = num_read(test_in);
    fclose(test_in);
    
    if (exp_error != long_number_errno)
    {
        num_free(num);
        return 0;
    }
    
    if (long_number_errno != ERROR_OK)
        return 1;
    
    if (num.n != exp_n || num.is_negative != exp_negative)
    {
        num_free(num);
        return 0;
    }
    
    for (unsigned i = 0; i < num.n; i++)
        if (num.digits[i] != (unsigned)(exp_digits[exp_n - i - 1] - '0'))
        {
            num_free(num);
            return 0;
        }
        
    num_free(num);
    
    return 1;
}

void test_read()
{
    begin_test_group("num_read");
    
    verify(check_num_read("+123", ERROR_OK, 0, 3, "123"));
    verify(check_num_read("-123", ERROR_OK, 1, 3, "123"));
    verify(check_num_read("+0asdf100", ERROR_OK, 0, 1, "0"));
    verify(check_num_read("+asdf", ERROR_INVALID_FORMAT, 0, 1, 0));
    verify(check_num_read("a100", ERROR_INVALID_FORMAT, 0, 1, 0));
    verify(check_num_read("", ERROR_INVALID_FORMAT, 0, 1, 0));
    
    const char* longNum = "1005001005001000500";
    verify(check_num_read(longNum, ERROR_OK, 0, strlen(longNum), longNum));
    
    num_read(0);
    verify(long_number_errno == ERROR_INVALID_ARGUMENT);
    
    end_test_group();
}

char check_num_write(const char* read_string, num_error_t expected_error, const char* expected_string)
{
    FILE* test_in = fopen("tmp.txt", "w");
    assert(test_in);
    fprintf(test_in, "%s", read_string);
    fclose(test_in);
    
    test_in = fopen("tmp.txt", "r");
    assert(test_in);
    number num = num_read(test_in);
    fclose(test_in);
    
    if (long_number_errno != ERROR_OK)
    {
        num_free(num);
        return 0;
    }
    
    test_in = fopen("tmp.txt", "w");
    assert(test_in);
    num_write(test_in, num);
    fclose(test_in);
    
    if (long_number_errno != expected_error)
    {
        num_free(num);
        return 0;
    }
    
    if (expected_error != ERROR_OK)
    {
        num_free(num);
        return 1;
    }
    
    test_in = fopen("tmp.txt", "r");
    assert(test_in);
    char* written_string = 0;
    fscanf(test_in, "%ms", &written_string);
    fclose(test_in);
    
    assert(written_string);
    char ok = !strcmp(written_string, expected_string);
    
    free(written_string);
    num_free(num);
    
    return ok;
}

void test_write()
{
    begin_test_group("num_write");
    
    verify(check_num_write("123", ERROR_OK, "123"));
    verify(check_num_write("000123", ERROR_OK, "123"));
    verify(check_num_write("+0000", ERROR_OK, "0"));
    verify(check_num_write("-00", ERROR_OK, "0"));
    verify(check_num_write("-123", ERROR_OK, "-123"));
    verify(check_num_write("1", ERROR_OK, "1"));
    verify(check_num_write("-2", ERROR_OK, "-2"));
    
    num_write(0, get_null_num());
    verify(long_number_errno == ERROR_INVALID_ARGUMENT);
    
    FILE* valid_out = fopen("tmp.txt", "w");
    num_write(valid_out, get_null_num());
    fclose(valid_out);
    verify(long_number_errno == ERROR_INVALID_ARGUMENT);
    
    end_test_group();
}

void run_unit_tests()
{
    test_utility_functions();
    test_read();
    test_write();
}

int main()
{
    run_unit_tests();
    
    if (GROUPS_FAILED > 0)
        return 1;
    
    return 0;
}
