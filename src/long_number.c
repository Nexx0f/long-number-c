#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
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
    
    unsigned n = strlen(str);
    
    for (unsigned i = 0; i < n; i++)
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
    for (unsigned i = 0; i < n; i++)
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

// utility routine, does not alter error codes
number shrink_leading_zeros(number a)
{
    while (a.n > 1 && a.digits[a.n - 1] == 0)
        a.n--;
    
    a.digits = realloc(a.digits, a.n * sizeof(a.digits[0]));
    return a;
}

unsigned unsigned_max(unsigned a, unsigned b)
{
    if (a > b)
        return a;
    else
        return b;
}

number num_add_unisign(number a, number b)
{
    number res;
    res.is_negative = a.is_negative;
    res.n = unsigned_max(a.n, b.n) + 1;
    res.digits = calloc(res.n, sizeof(res.digits[0]));
    
    int carry = 0;
    for (unsigned i = 0; i < res.n; i++)
    {
        if (i < a.n) carry += a.digits[i];
        if (i < b.n) carry += b.digits[i];
        
        res.digits[i] = carry % 10;
        carry /= 10;
    }
    
    num_return(ERROR_OK, shrink_leading_zeros(res));
}

number num_subtract_lower(number a, number b)
{
    number res;
    res.is_negative = 0;
    res.n = a.n;
    res.digits = calloc(res.n, sizeof(res.digits[0]));
    
    char is_carry = 0;
    for (unsigned i = 0; i < a.n; i++)
    {
        int digit = a.digits[i];
        
        if (is_carry && digit > 0)
        {
            is_carry = 0;
            digit--;
        }
        
        if (is_carry)
            digit += 9;
        
        int needSub = i < b.n ? b.digits[i] : 0;
        
        if (needSub > digit)
        {
            digit += 10;
            is_carry = 1;
        }
        
        res.digits[i] = digit - needSub;
    }
    
    num_return(ERROR_OK, shrink_leading_zeros(res));
}

number num_add(number a, number b)
{
    if (num_is_null(a) || num_is_null(b))
        num_return(ERROR_INVALID_ARGUMENT, get_null_num());
    
    if (a.is_negative == b.is_negative)
        return num_add_unisign(a, b);

    if (a.is_negative)
    {
        a.is_negative = !a.is_negative;
        b.is_negative = !b.is_negative;
        number x = num_add(a, b);
        x.is_negative = !x.is_negative;
        return x;
    }
    
    // a >= 0
    b.is_negative = 0;
    
    if (num_compare(a, b) >= 0)
        return num_subtract_lower(a, b);
    
    number res = num_subtract_lower(b, a);
    res.is_negative = 1;
    num_return(ERROR_OK, res);
}

number num_sub(number a, number b)
{
    if (num_is_null(a) || num_is_null(b))
        num_return(ERROR_INVALID_ARGUMENT, get_null_num());
    
    b.is_negative = !b.is_negative;
    return num_add(a, b);
}

number num_mul(number a, number b)
{
    if (num_is_null(a) || num_is_null(b))
        num_return(ERROR_INVALID_ARGUMENT, get_null_num());
    
    number res;
    
    res.is_negative = (a.is_negative ? 1 : 0) ^ (b.is_negative ? 1 : 0);
    res.n = a.n + b.n + 1;
    res.digits = calloc(res.n, sizeof(res.digits[0]) * res.n);
    
    for (unsigned i = 0; i < a.n; i++)
    {
        int carry = 0;
        
        for (unsigned j = i; j < res.n; j++)
        {
            carry += res.digits[j];
            
            if (j - i < b.n)
                carry += b.digits[j - i] * a.digits[i];
            
            res.digits[j] = carry % 10;
            carry /= 10;
        }
    }
    
    num_return(ERROR_OK, shrink_leading_zeros(res));
}

// i -> i + by
void shift(number a, int by)
{
    int from = by == 1 ? a.n - 1 : 0;
    int to = by == 1 ? -1 : (int)a.n;
    for (int i = from; i != to; i -= by)
        a.digits[i] = (i - by >= 0 && i - by < (int)a.n) ? a.digits[i - by] : 0;
}

number num_copy(number a)
{
    number b = a;
    b.digits = malloc(a.n * sizeof(a.digits[0]));
    memcpy(b.digits, a.digits, b.n * sizeof(b.digits[0]));
    return b;
}

void num_div(number a, number b, number* quotient, number* remainder)
{
    *quotient = get_null_num();
    *remainder = get_null_num();
    
    if (num_is_null(a) || num_is_null(b))
        num_return(ERROR_INVALID_ARGUMENT, VOID);
    
    if (num_is_zero(b))
        num_return(ERROR_DIVISION_BY_ZERO, VOID);
    
    char res_negative = (a.is_negative ? 1 : 0) ^ (b.is_negative ? 1 : 0);
    
    a.is_negative = b.is_negative = 0;
    
    number r = num_copy(a);
    
    number q = a;
    q.digits = calloc(q.n, sizeof(q.digits[0]));
    
    number bPower;
    bPower.n = a.n + 1;
    bPower.digits = calloc(bPower.n, sizeof(bPower.digits[0]));
    bPower.is_negative = 0;
    memcpy(bPower.digits, b.digits, sizeof(bPower.digits[0]) * b.n);
    
    int nShifts = 0;
    while (num_compare(r, bPower) >= 0)
    {
        shift(bPower, 1);
        nShifts++;
    }
    
    while (nShifts > 0)
    {
        shift(bPower, -1);
        nShifts--;
        
        int digit = 0;
        while (num_compare(r, bPower) >= 0)
        {
            number rSub = num_sub(r, bPower);
            num_free(r);
            r = rSub;
            digit++;
        }
        
        q.digits[nShifts] = digit;
    }
    
    q.is_negative = r.is_negative = res_negative;
    
    *quotient = shrink_leading_zeros(q);
    *remainder = shrink_leading_zeros(r);
    
    num_free(bPower);
    
    num_return(ERROR_OK, VOID);
}
