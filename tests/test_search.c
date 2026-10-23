#include <assert.h>
#include "tests.h"
#include "client.h"
#include "site.h"
#include "hotel.h"
#include "search.h"

void test_search(void)
{
    T_Hotel *hotel    = createHotel("Test");
    T_Site  *bordeaux = createSite("Bordeaux");
    T_Site  *paris    = createSite("Paris");
    T_Site  *lille    = createSite("Lille");
    addSite(hotel, bordeaux);
    addSite(hotel, paris);
    addSite(hotel, lille);
    addClient(bordeaux, createClient("DUVAL", "Magali", 40, 38));
    addClient(bordeaux, createClient("DUPOND", "Agathe", 23, 12));
    addClient(paris, createClient("DURAND", "Antoine", 42, 15));
    addClient(paris, createClient("DUPOND", "Francois", 35, 29));
    addClient(paris, createClient("MARTIN", "Alain", 42, 112));
    addClient(lille, createClient("DUPONT", "Amelie", 28, 17));

    /* should be sorted even across sites, "du" matches DU..., MARTIN shouldnt be there */
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
    assert(!findClients(NULL, "D"));
    searchClients(hotel, "DUP");
    searchClients(hotel, "XYZ");

    freeHotel(hotel);
    printf("test_search passed.\n");
}
