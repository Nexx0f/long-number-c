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
    
    // num_is_zero
    a = num_parse("0");
    check_ok();
    verify(num_is_zero(a));
    num_free(a);
    
    a = num_parse("-0");
    check_ok();
    verify(num_is_zero(a));
    num_free(a);
    
    a = num_parse("+000");
    check_ok();
    verify(num_is_zero(a));
    num_free(a);
    
    a = num_parse("-1");
    check_ok();
    verify(!num_is_zero(a));
    num_free(a);
    
    a = num_parse("-10");
    check_ok();
    verify(!num_is_zero(a));
    num_free(a);
    
    num_is_zero(get_null_num());
    verify(long_number_errno == ERROR_INVALID_ARGUMENT);
    
    end_test_group();
}

char check_num_read(const char* str, num_error_t exp_error,
                    char exp_negative, unsigned exp_n, const char* exp_digits)
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

char check_num_write(const char* read_string,
                     num_error_t expected_error, const char* expected_string)
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

char check_num_compare(const char* a, const char* b, int expected)
{
    number num_a = num_parse(a);
    number num_b = num_parse(b);
    
    int result = num_compare(num_a, num_b);
    assert(long_number_errno == ERROR_OK);
    
    num_free(num_a);
    num_free(num_b);
    
    return result == expected;
}

void test_cmp()
{
    begin_test_group("num_compare");
    
    number a = num_parse("1");
    
    num_compare(a, get_null_num());
    verify(long_number_errno == ERROR_INVALID_ARGUMENT);
    
    num_compare(get_null_num(), a);
    verify(long_number_errno == ERROR_INVALID_ARGUMENT);
    
    num_free(a);
    
    verify(check_num_compare("1", "1", 0));
    verify(check_num_compare("1", "0", 1));
    verify(check_num_compare("-1", "0", -1));
    verify(check_num_compare("0", "1", -1));
    verify(check_num_compare("0", "-1", 1));
    verify(check_num_compare("0", "0", 0));
    verify(check_num_compare("100", "10", 1));
    verify(check_num_compare("10", "100", -1));
    verify(check_num_compare("-100", "-10", -1));
    verify(check_num_compare("-10", "-100", 1));
    verify(check_num_compare("12345", "12345", 0));
    verify(check_num_compare("12345", "12346", -1));
    verify(check_num_compare("12345", "12344", 1));
    verify(check_num_compare("1", "2", -1));
    verify(check_num_compare("1", "-2", 1));
    verify(check_num_compare("-1", "2", -1));
    verify(check_num_compare("-1", "-2", 1));
    verify(check_num_compare("0010", "000010", 0));
    
    end_test_group();
}

char check_num_op(const char* a, char op, const char* b, const char* c)
{
    number num_a = num_parse(a);
    assert(long_number_errno == ERROR_OK);
    
    number num_b = num_parse(b);
    assert(long_number_errno == ERROR_OK);
    
    number num_c = num_parse(c);
    assert(long_number_errno == ERROR_OK);
    
    number op_res = get_null_num(), junk = get_null_num();
    
    switch (op)
    {
        case '+': op_res = num_add(num_a, num_b); break;
        case '-': op_res = num_sub(num_a, num_b); break;
        case '*': op_res = num_mul(num_a, num_b); break;
        case '/': num_div(num_a, num_b, &op_res, &junk); break;
        case '%': num_div(num_a, num_b, &junk, &op_res); break;
        default: assert(!"invalid op");
    }
    
    char eq = num_compare(num_c, op_res) == 0;
    
    print_tabs();
    printf("result: ");
    num_write(stdout, op_res);
    printf("\n");
    
    num_free(num_a);
    num_free(num_b);
    num_free(num_c);
    num_free(op_res);
    num_free(junk);
    
    return eq;
}

void test_ops()
{
    begin_test_group("arithmetical operations");
    
    number a = num_parse("0");
    
    num_add(a, get_null_num());
    verify(long_number_errno == ERROR_INVALID_ARGUMENT);
    num_sub(a, get_null_num());
    verify(long_number_errno == ERROR_INVALID_ARGUMENT);
    num_mul(a, get_null_num());
    verify(long_number_errno == ERROR_INVALID_ARGUMENT);
    num_add(get_null_num(), a);
    verify(long_number_errno == ERROR_INVALID_ARGUMENT);
    num_sub(get_null_num(), a);
    verify(long_number_errno == ERROR_INVALID_ARGUMENT);
    num_mul(get_null_num(), a);
    verify(long_number_errno == ERROR_INVALID_ARGUMENT);
    
    number q, r;
    num_div(a, get_null_num(), &q, &r);
    verify(long_number_errno == ERROR_INVALID_ARGUMENT);
    num_div(get_null_num(), a, &q, &r);
    verify(long_number_errno == ERROR_INVALID_ARGUMENT);
    num_div(a, a, &q, &r);
    verify(long_number_errno == ERROR_DIVISION_BY_ZERO);
    
    num_free(a);
    
    verify(check_num_op("10", '+', "10", "20"));
    verify(check_num_op("10", '-', "10", "0"));
    verify(check_num_op("-10", '+', "-10", "-20"));
    verify(check_num_op("-10", '+', "20", "10"));
    verify(check_num_op("-10", '+', "20", "10"));
    verify(check_num_op("11", '-', "2", "9"));
    verify(check_num_op("1010", '-', "101", "909"));
    verify(check_num_op("100000", '-', "1", "99999"));
    verify(check_num_op("11", '+', "9", "20"));
    
    verify(check_num_op("9", '*', "9", "81"));
    verify(check_num_op("-23423", '*', "-234234", "5486462982"));
    verify(check_num_op("10", '*', "-10", "-100"));
    verify(check_num_op("10", '*', "-10", "-100"));
    verify(check_num_op("10", '*', "0", "0"));
    verify(check_num_op("0", '*', "-10", "0"));
    
    verify(check_num_op("10", '/', "2", "5"));
    verify(check_num_op("5486462982", '/', "-23423", "-234234"));
    verify(check_num_op("100500", '/', "7", "14357"));
    verify(check_num_op("1201", '/', "3", "400"));
    verify(check_num_op("-1000", '/', "-100", "10"));
    verify(check_num_op("-3", '/', "2", "-1"));
    verify(check_num_op("3", '/', "-2", "-1"));
    
    verify(check_num_op("10", '%', "2", "0"));
    verify(check_num_op("5486462982", '%', "-23423", "0"));
    verify(check_num_op("100500", '%', "7", "1"));
    verify(check_num_op("1201", '%', "3", "1"));
    verify(check_num_op("-1000", '%', "-100", "0"));
    verify(check_num_op("-3", '%', "2", "-1"));
    verify(check_num_op("3", '%', "-2", "-1"));
    verify(check_num_op("-5", '%', "-3", "2"));
    verify(check_num_op("-5", '%', "3", "-2"));
    
    end_test_group();
}

void run_unit_tests()
{
    test_utility_functions();
    test_read();
    test_write();
    test_cmp();
    test_ops();
}

int main()
{
    run_unit_tests();
    
    if (GROUPS_FAILED > 0)
        return 1;
    
    return 0;
}
