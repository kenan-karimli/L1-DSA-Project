#include <assert.h>
#include "tests.h"
#include "client.h"
#include "site.h"
#include "hotel.h"
#include "file.h"

/* Runs from the repository root (make test), where data/bookings.txt lives */
void test_file(void)
{
    T_Hotel *hotel = createHotel("Test");
    processingfile(hotel);

    const char *cities[] = {"Bordeaux", "Lille", "Lyon", "Paris", "Toulouse"};
    const int   counts[] = {3, 2, 1, 3, 1};
    T_Site *temp = hotel->first;
    for (int i = 0; i < 5; i++, temp = temp->next) {
        assert(temp);
        assert(strcmp(temp->city, cities[i]) == 0);
        assert(temp->number_clients == counts[i]);
    }
    assert(!temp);

    T_Site *bordeaux = findSite(hotel, "Bordeaux");
    assert(strcmp(bordeaux->first->last_name, "ANTOINE") == 0);

    processingfile(hotel);                            /* same file again: every line is a duplicate */
    assert(findSite(hotel, "Paris")->number_clients == 3);

    processingfile(NULL);
    freeHotel(hotel);
    printf("test_file passed.\n");
}
