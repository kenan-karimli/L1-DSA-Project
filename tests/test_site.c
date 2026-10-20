#include <assert.h>
#include "tests.h"
#include "client.h"
#include "site.h"
#include "hotel.h"

void test_site(void)
{
    T_Hotel *hotel = createHotel("Test");
    T_Site  *paris = createSite("Paris");
    assert(paris && paris->number_clients == 0 && !paris->first && strcmp(paris->city, "Paris") == 0);

    assert(addSite(hotel, paris));
    assert(addSite(hotel, createSite("Bordeaux")));   /* new head */
    assert(addSite(hotel, createSite("Toulouse")));   /* tail */
    assert(addSite(hotel, createSite("Lille")));      /* middle */

    T_Site *again = createSite("Paris");
    assert(!addSite(hotel, again));                   /* same city twice */
    free(again->city);
    free(again);

    const char *order[] = {"Bordeaux", "Lille", "Paris", "Toulouse"};
    T_Site *temp = hotel->first;
    for (int i = 0; i < 4; i++, temp = temp->next)
        assert(strcmp(temp->city, order[i]) == 0);
    assert(!temp);

    assert(findSite(hotel, "Paris") == paris);
    assert(!findSite(hotel, "Lyon"));

    addClient(paris, createClient("Martin", "Alain", 42, 112));
    addClient(paris, createClient("Dupond", "Charles", 28, 125));
    assert(deleteSite(hotel, "Paris"));               /* middle, with clients: freed (checked by make debug) */
    assert(deleteSite(hotel, "Bordeaux"));            /* head */
    assert(deleteSite(hotel, "Toulouse"));            /* tail */
    assert(!deleteSite(hotel, "Paris"));
    assert(hotel->first && !hotel->first->next && strcmp(hotel->first->city, "Lille") == 0);

    freeHotel(hotel);
    printf("test_site passed.\n");
}
