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

    /* chain file: empty site has to survive, and clients must be exactly the same */
    addSite(hotel, createSite("Nantes"));
    const char *chain = "build/test_chain_file.txt";
    assert(saveChainFile(hotel, chain, "pbkdf2$1$ab$cd") == 10);
    copy = createHotel("Copy");
    loadBookings(copy, chain);
    for (a = hotel->first, b = copy->first; a && b; a = a->next, b = b->next) {
        assert(strcmp(a->city, b->city) == 0);
        assert(a->number_clients == b->number_clients);
        T_Client *x = a->first, *y = b->first;
        for (; x && y; x = x->next, y = y->next) {
            assert(compareNames(x->last_name, x->first_name, y->last_name, y->first_name) == 0);
            assert(x->age == y->age && x->num_room == y->num_room);
        }
        assert(!x && !y);
    }
    assert(!a && !b);
    assert(findSite(copy, "Nantes") && findSite(copy, "Nantes")->number_clients == 0);
    assert(saveChainFile(hotel, chain, NULL) == -1);
    assert(saveChainFile(hotel, chain, "") == 10);   /* no file password */
    remove(chain);
    freeHotel(copy);

    /* # lines are not bookings: #HOTEL and unknown ones are ignored, #SITE makes a site */
    FILE *f = fopen(chain, "w");
    fprintf(f, "#HOTEL;Name;login;hash\n# just a comment\n#SITE;Rome\n"
               "LastName;FirstName;Age;Room Number;City\nDOE;John;30;7;Oslo\n");
    fclose(f);
    copy = createHotel("Copy");
    loadBookings(copy, chain);
    assert(strcmp(copy->first->city, "Oslo") == 0 && copy->first->number_clients == 1);
    assert(strcmp(copy->first->next->city, "Rome") == 0 && copy->first->next->number_clients == 0);
    assert(!copy->first->next->next);
    remove(chain);
    freeHotel(copy);

    processingfile(NULL);
    freeHotel(hotel);
    printf("test_file passed.\n");
}
