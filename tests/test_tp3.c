#include <assert.h>
#include "tp3.h"

/* Checks that a site has exactly n clients in the expected order (last, first, last, first...). */
static void checkOrder(T_Site *site, const char *expected[], int n)
{
    T_Client *temp = site->first;
    for (int i = 0; i < n; i++, temp = temp->next) {
        assert(temp);
        assert(strcmp(temp->last_name, expected[2 * i]) == 0);
        assert(strcmp(temp->first_name, expected[2 * i + 1]) == 0);
    }
    assert(!temp);
    assert(site->number_clients == n);
}

static void testClients(void)
{
    T_Hotel *hotel = createHotel("Test");
    T_Site  *site  = createSite("Paris");
    addSite(hotel, site);
    assert(!createClient(NULL, "x", 1, 1));

    assert(addClient(site, createClient("Martin", "Alain", 42, 112)));   /* empty list */
    assert(addClient(site, createClient("Dupond", "Michele", 46, 56)));  /* new head */
    assert(addClient(site, createClient("Dupond", "Charles", 28, 125))); /* same last name */
    assert(addClient(site, createClient("Zola", "Emile", 30, 1)));       /* tail */
    assert(addClient(site, createClient("Leroy", "Anne", 30, 2)));       /* middle */
    const char *order[] = {"Dupond", "Charles", "Dupond", "Michele", "Leroy", "Anne", "Martin", "Alain", "Zola", "Emile"};
    checkOrder(site, order, 5);

    T_Client *dup = createClient("Leroy", "Anne", 50, 3);
    assert(!addClient(site, dup));                                       /* same person twice */
    freeClients(dup);

    assert(deleteClient(site, "Leroy", "Anne"));                         /* middle */
    assert(deleteClient(site, "Dupond", "Charles"));                     /* head */
    assert(deleteClient(site, "Zola", "Emile"));                         /* tail */
    assert(!deleteClient(site, "Nobody", "Here"));
    const char *left[] = {"Dupond", "Michele", "Martin", "Alain"};
    checkOrder(site, left, 2);

    displaySite(site);
    freeHotel(hotel);
}

static void testSites(void)
{
    T_Hotel *hotel = createHotel("Test");
    T_Site  *paris = createSite("Paris");
    assert(addSite(hotel, paris));
    assert(addSite(hotel, createSite("Bordeaux")));  /* new head */
    assert(addSite(hotel, createSite("Toulouse")));  /* tail */
    assert(addSite(hotel, createSite("Lille")));     /* middle */

    T_Site *again = createSite("Paris");
    assert(!addSite(hotel, again));                  /* same city twice */
    free(again->city);
    free(again);

    const char *order[] = {"Bordeaux", "Lille", "Paris", "Toulouse"};
    T_Site *temp = hotel->first;
    for (int i = 0; i < 4; i++, temp = temp->next)
        assert(strcmp(temp->city, order[i]) == 0);
    assert(!temp);
    assert(findSite(hotel, "Paris") == paris && !findSite(hotel, "Lyon"));
    displayHotel(hotel);

    addClient(paris, createClient("Martin", "Alain", 42, 112));
    assert(deleteSite(hotel, "Paris"));              /* middle, with a client */
    assert(deleteSite(hotel, "Bordeaux"));           /* head */
    assert(deleteSite(hotel, "Toulouse"));           /* tail */
    assert(!deleteSite(hotel, "Paris"));
    assert(strcmp(hotel->first->city, "Lille") == 0 && !hotel->first->next);
    freeHotel(hotel);
}

static void testSearch(void)
{
    T_Hotel *hotel = createHotel("Test");
    addBooking(hotel, "DUVAL", "Magali", 40, 38, "Bordeaux");
    addBooking(hotel, "DUPOND", "Agathe", 23, 12, "Bordeaux");
    addBooking(hotel, "DURAND", "Antoine", 42, 15, "Paris");
    addBooking(hotel, "DUPOND", "Francois", 35, 29, "Paris");
    addBooking(hotel, "MARTIN", "Alain", 42, 112, "Paris");
    addBooking(hotel, "DUPONT", "Amelie", 28, 17, "Lille");

    /* sorted across all sites, MARTIN must not be there */
    const char *expected[] = {"DUPOND", "Agathe", "Bordeaux", "DUPOND", "Francois", "Paris",
                              "DUPONT", "Amelie", "Lille", "DURAND", "Antoine", "Paris",
                              "DUVAL", "Magali", "Bordeaux"};
    T_Result *results = findClients(hotel, "du");
    T_Result *temp    = results;
    for (int i = 0; i < 5; i++, temp = temp->next) {
        assert(temp);
        assert(strcmp(temp->client->last_name, expected[3 * i]) == 0);
        assert(strcmp(temp->client->first_name, expected[3 * i + 1]) == 0);
        assert(strcmp(temp->city, expected[3 * i + 2]) == 0);
    }
    assert(!temp);
    freeResults(results);

    assert(!findClients(hotel, "XYZ"));
    searchClients(hotel, "DUP");
    freeHotel(hotel);
}

/* Must run from the project folder so data/bookings.txt is found. */
static void testFile(void)
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

    processingfile(hotel);                           /* all duplicates: nothing added */
    assert(findSite(hotel, "Paris")->number_clients == 3);
    freeHotel(hotel);
}

int main(void)
{
    testClients();
    testSites();
    testSearch();
    testFile();
    printf("All tp3 tests passed.\n");
    return 0;
}
