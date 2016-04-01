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
    int n = strlen(str);
    
    for (int i = 0; i < n; i++)
        if (!(isdigit(str[i]) || (i == 0 && (str[i] == '+' || str[i] == '-'))))
            num_return(ERROR_INVALID_FORMAT, get_null_num());
        
    if (n == 0 || (!isdigit(str[0]) && n == 1))
        num_return(ERROR_INVALID_FORMAT, get_null_num());
    
    number num;
    
    if (str[0] == '+' || str[0] == '-')
    {
        num.is_negative = str[0] == '-';
        n--;
        str++;
    }
    
    num.n = n - 1;
    if (num.n == 0)
        num.n++;
    
    num.digits = calloc(num.n, sizeof(num.digits[0]));
    for (int i = 0; i < n; i++)
        num.digits[i] = str[n - i - 1] - '0';
    
    num_return(ERROR_OK, num);
}
