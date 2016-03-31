#ifndef LONG_NUMBER_H
#define LONG_NUMBER_H

#include <stdio.h>

typedef enum
{
    // operation completed successfully
    ERROR_OK = 0,
    
    // failed to parse number
    ERROR_INVALID_NUMBER_FORMAT,
    
    // division by zero
    ERROR_DIVISION_BY_ZERO,
    
    // file operation failure
    ERROR_IO
} error_t;

extern error_t long_number_errno;

// n >= 1 always
// negative zero allowed
// digits can be zero -> null number
typedef struct
{
    int* digits;
    int n;
    char isNegative;
} number;

char num_is_valid(number num);

number num_parse(const char* string);

number num_read(FILE* input);
void num_write(FILE* output, number num);

void num_free(number num);

// sign(a-b)
int num_compare(number a, number b);

number num_add(number a, number b);
number num_sub(number a, number b);
number num_mul(number a, number b);
void num_div(number a, number b, number* quotient, number* remainder);

#endif // LONG_NUMBER_H
