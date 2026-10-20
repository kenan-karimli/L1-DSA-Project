#include <assert.h>
#include "tests.h"
#include "client.h"
#include "site.h"
#include "hotel.h"

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

void test_client(void)
{
    char *s = copyString("Paris");
    assert(s && strcmp(s, "Paris") == 0);
    free(s);
    assert(compareNames("Dupond", "Charles", "Dupond", "Michele") < 0);
    assert(compareNames("Martin", "Alain", "Dupond", "Michele") > 0);
    assert(compareNames("Martin", "Alain", "Martin", "Alain") == 0);

    T_Client *c = createClient("Martin", "Alain", 42, 112);
    assert(c && c->age == 42 && c->num_room == 112 && !c->next);
    assert(!createClient(NULL, "x", 1, 1));

    T_Site *site = createSite("Paris");
    assert(addClient(site, c));                                            /* empty list */
    assert(addClient(site, createClient("Dupond", "Michele", 46, 56)));    /* new head */
    assert(addClient(site, createClient("Dupond", "Charles", 28, 125)));   /* same last name, new head */
    assert(addClient(site, createClient("Zola", "Emile", 30, 1)));         /* tail */
    assert(addClient(site, createClient("Leroy", "Anne", 30, 2)));         /* middle */
    const char *order[] = {"Dupond", "Charles", "Dupond", "Michele", "Leroy", "Anne", "Martin", "Alain", "Zola", "Emile"};
    checkOrder(site, order, 5);

    T_Client *dup = createClient("Leroy", "Anne", 50, 3);
    assert(!addClient(site, dup));                                         /* duplicate name rejected */
    freeClients(dup);
    assert(!addClient(NULL, c) && !addClient(site, NULL));

    assert(deleteClient(site, "Leroy", "Anne"));                           /* middle */
    assert(deleteClient(site, "Dupond", "Charles"));                       /* head */
    assert(deleteClient(site, "Zola", "Emile"));                           /* tail */
    assert(!deleteClient(site, "Nobody", "Here"));
    const char *left[] = {"Dupond", "Michele", "Martin", "Alain"};
    checkOrder(site, left, 2);

    freeClients(site->first);
    free(site->city);
    free(site);
    printf("test_client passed.\n");
}
