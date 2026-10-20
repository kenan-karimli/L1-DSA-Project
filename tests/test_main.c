#include <stdio.h>
#include "tests.h"

int main(void)
{
    test_client();
    test_site();
    test_hotel();
    test_search();
    test_file();
    printf("All tests passed.\n");
    return 0;
}
