#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "long_number.h"

num_error_t long_number_errno = ERROR_OK;

#define VOID
#define num_return(errorcode, returnvalue) \
    { long_number_errno = errorcode; return returnvalue; }
    
number get_null_num()
{
    number null;
    null.is_negative = 0;
    null.n = 0;
    null.digits = 0;
    return null;
}

char num_is_null(number num)
{
    num_return(ERROR_OK, num.digits == 0);
}

number num_parse(const char* str)
{
    if (!str)
        num_return(ERROR_INVALID_ARGUMENT, get_null_num());
    
    int n = strlen(str);
    
    for (int i = 0; i < n; i++)
        if (!(isdigit(str[i]) || (i == 0 && (str[i] == '+' || str[i] == '-'))))
            num_return(ERROR_INVALID_FORMAT, get_null_num());
        
    if (n == 0 || (!isdigit(str[0]) && n == 1))
        num_return(ERROR_INVALID_FORMAT, get_null_num());
    
    number num = get_null_num();
    
    if (str[0] == '+' || str[0] == '-')
    {
        num.is_negative = str[0] == '-';
        n--;
        str++;
    }
    
    num.n = n;
    
    num.digits = calloc(num.n, sizeof(num.digits[0]));
    for (int i = 0; i < n; i++)
        num.digits[i] = str[n - i - 1] - '0';
    
    num_return(ERROR_OK, num);
}

number num_free(number num)
{
    if (num.digits)
        free(num.digits);
    
    num_return(ERROR_OK, get_null_num());
}

number num_read(FILE* input)
{   
    if (!input)
        num_return(ERROR_INVALID_ARGUMENT, get_null_num());
    
    number num = get_null_num();
    
    int front = fgetc(input);
    if (front == '+' || front == '-')
    {
        num.is_negative = front == '-';
        front = fgetc(input);
    }
    
    if (!isdigit(front))
        num_return(ERROR_INVALID_FORMAT, get_null_num());
    
    unsigned buffer_size = 10;
    char* buffer = calloc(buffer_size, sizeof(char));
    unsigned buffer_pos = 0;
    
    do
    {
        if (buffer_pos == buffer_size)
        {
            buffer_size *= 2;
            buffer = realloc(buffer, buffer_size);
        }
        
        buffer[buffer_pos++] = front;
        
        front = fgetc(input);
    }
    while (front != EOF && isdigit(front));
    
    if (front != EOF)
        ungetc(front, input);
    
    num.n = buffer_pos;
    num.digits = calloc(buffer_pos, sizeof(num.digits[0]));
    for (unsigned i = 0; i < buffer_pos; i++)
        num.digits[buffer_pos - i - 1] = buffer[i] - '0';
    
    free(buffer);
    
    num_return(ERROR_OK, num);
}

void num_write(FILE* out, number num)
{
    if (!out || num_is_null(num))
        num_return(ERROR_INVALID_ARGUMENT, VOID);
    
    int nonzero = num.n - 1;
    while (nonzero > 0 && !num.digits[nonzero])
        nonzero--;
    
    if (nonzero == 0 && num.digits[nonzero] == 0)
    {
        fputc('0', out);
        num_return(ERROR_OK, VOID);
    }
    
    if (num.is_negative)
        fputc('-', out);
    
    for (int i = nonzero; i >= 0; i--)
        fputc('0' + num.digits[i], out);
    
    num_return(ERROR_OK, VOID);
}

char num_is_zero(number x)
{
    if (num_is_null(x))
        num_return(ERROR_INVALID_ARGUMENT, 0);
    
    unsigned nonzero = x.n - 1;
    while (nonzero > 0 && x.digits[nonzero] == 0)
        nonzero--;
    
    num_return(ERROR_OK, x.digits[nonzero] == 0);
}

int num_compare(number a, number b)
{
    if (num_is_null(a) || num_is_null(b))
        num_return(ERROR_INVALID_ARGUMENT, 0);
    
    unsigned aNonzero = a.n - 1, bNonzero = b.n - 1;
    
    while (aNonzero > 0 && a.digits[aNonzero] == 0) aNonzero--;
    while (bNonzero > 0 && b.digits[bNonzero] == 0) bNonzero--;
        
    char aZero = aNonzero == 0 && a.digits[aNonzero] == 0;
    char bZero = bNonzero == 0 && b.digits[bNonzero] == 0;
    
    long_number_errno = ERROR_OK;
        
    if (aZero && bZero) return 0;
    
    if (aZero) return b.is_negative ? 1 : -1;
    if (bZero) return a.is_negative ? -1 : 1;
    
    if (a.is_negative != b.is_negative) return a.is_negative ? -1 : 1;
    
    int no_sign_result = 0;
    
    if (aNonzero == bNonzero)
    {
        for (int i = aNonzero; i >= 0; i--)
            if (a.digits[i] != b.digits[i])
            {
                no_sign_result = a.digits[i] > b.digits[i] ? 1 : -1;
                break;
            }
    }
    else
    {
        no_sign_result = aNonzero > bNonzero ? 1 : -1;
    }
    
    return no_sign_result * (a.is_negative ? -1 : 1);
}
