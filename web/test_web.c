#include <assert.h>
#include "web.h"

/* Checks that two chains have the same sites and clients. */
static void checkSame(T_Hotel *h1, T_Hotel *h2)
{
    T_Site *a = h1->first, *b = h2->first;
    for (; a && b; a = a->next, b = b->next) {
        assert(strcmp(a->city, b->city) == 0 && a->number_clients == b->number_clients);
        T_Client *x = a->first, *y = b->first;
        for (; x && y; x = x->next, y = y->next) {
            assert(compareNames(x->last_name, x->first_name, y->last_name, y->first_name) == 0);
            assert(x->age == y->age && x->num_room == y->num_room);
        }
        assert(!x && !y);
    }
    assert(!a && !b);
}

int main(void)
{
    char *path = "build/test_web.txt";
    T_Hotel *hotel = createHotel("Test");
    processingfile(hotel);

    /* saveHotel + loadBookings give the same chain back */
    assert(saveHotel(hotel, path) == 10);
    T_Hotel *copy = createHotel("Copy");
    loadBookings(copy, path);
    checkSame(hotel, copy);
    freeHotel(copy);
    assert(saveHotel(NULL, path) == -1);
    assert(saveHotel(hotel, "no_such_dir/x.txt") == -1);

    /* chain file keeps empty sites */
    addSite(hotel, createSite("Nantes"));
    assert(saveChainFile(hotel, path, "pbkdf2$1$ab$cd") == 10);
    copy = createHotel("Copy");
    loadBookings(copy, path);
    checkSame(hotel, copy);
    freeHotel(copy);
    assert(saveChainFile(hotel, path, NULL) == -1);

    /* '#' lines: #SITE makes a site, the others are ignored */
    FILE *f = fopen(path, "w");
    fprintf(f, "#HOTEL;Name;hash\n# comment\n#SITE;Rome\n"
               "LastName;FirstName;Age;Room Number;City\nDOE;John;30;7;Oslo\n");
    fclose(f);
    copy = createHotel("Copy");
    loadBookings(copy, path);
    assert(strcmp(copy->first->city, "Oslo") == 0 && copy->first->number_clients == 1);
    assert(strcmp(copy->first->next->city, "Rome") == 0 && copy->first->next->number_clients == 0);
    assert(!copy->first->next->next);
    freeHotel(copy);

    remove(path);
    freeHotel(hotel);
    printf("All web tests passed.\n");
    return 0;
}
