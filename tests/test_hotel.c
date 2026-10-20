#include <assert.h>
#include "tests.h"
#include "client.h"
#include "site.h"
#include "hotel.h"

void test_hotel(void)
{
    T_Hotel *hotel = createHotel("Ibis");
    assert(hotel && strcmp(hotel->name, "Ibis") == 0 && !hotel->first);
    assert(!createHotel(NULL));

    displayHotel(hotel);                              /* empty chain */
    T_Site *paris = createSite("Paris");
    addSite(hotel, paris);
    addSite(hotel, createSite("Bordeaux"));
    addClient(paris, createClient("Dupond", "Charles", 28, 125));
    addClient(paris, createClient("Martin", "Alain", 42, 112));
    displayHotel(hotel);
    displaySite(paris);
    displaySite(hotel->first);                        /* empty site */
    displaySite(NULL);

    freeHotel(hotel);                                 /* frees sites and clients: checked by make debug */
    freeHotel(NULL);
    printf("test_hotel passed.\n");
}
