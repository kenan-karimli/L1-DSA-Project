#include <assert.h>
#include "tests.h"
#include "client.h"
#include "site.h"
#include "hotel.h"
#include "file.h"

/* has to be run from the project folder (make test does that) or it wont find data/bookings.txt */
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

    processingfile(hotel);                            /* loading it again should add nothing (all duplicates) */
    assert(findSite(hotel, "Paris")->number_clients == 3);

    /* save, then load into another chain and check its the same */
    const char *saved = "build/test_saved_chain.txt";
    assert(saveHotel(hotel, saved) == 10);
    T_Hotel *copy = createHotel("Copy");
    loadBookings(copy, saved);
    T_Site *a = hotel->first, *b = copy->first;
    for (; a && b; a = a->next, b = b->next) {
        assert(strcmp(a->city, b->city) == 0);
        assert(a->number_clients == b->number_clients);
    }
    assert(!a && !b);
    assert(saveHotel(NULL, saved) == -1);
    assert(saveHotel(hotel, "no_such_dir/x.txt") == -1);
    remove(saved);
    freeHotel(copy);

    processingfile(NULL);
    freeHotel(hotel);
    printf("test_file passed.\n");
}
