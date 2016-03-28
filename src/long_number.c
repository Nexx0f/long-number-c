#include <stdio.h>
#include <stdlib.h>
#include "long_number.h"

error_t long_number_errno = ERROR_OK;

void testFunction()
{
    if (rand() % 2 == 1)
        printf("branch A\n");
    else
        printf("branch B\n");
}
