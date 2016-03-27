#include <stdio.h>
#include <stdlib.h>
#include "long_number.h"

void testFunction()
{
    if (rand() % 2 == 1)
        printf("branch A\n");
    else
        printf("branch B\n");
}
