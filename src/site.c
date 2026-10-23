#include "site.h"
#include "client.h"

T_Site *createSite(char *city)
{
    if (!city)
        return NULL;

    T_Site *newSite = malloc(sizeof(T_Site));
    if (!newSite)
        return NULL;

    newSite->city = copyString(city);
    if (!newSite->city) {
        free(newSite);
        return NULL;
    }
    newSite->number_clients = 0;
    newSite->first          = NULL;
    newSite->next           = NULL;
    return newSite;
}

/* sorted insert by city name, same city twice is refused.
 * if it returns 0 you have to free the site yourself */
int addSite(T_Hotel *hotel, T_Site *site)
{
    if (!hotel || !site)
        return 0;

    T_Site *prev = NULL;
    T_Site *temp = hotel->first;
    while (temp && strcmp(temp->city, site->city) < 0) {
        prev = temp;
        temp = temp->next;
    }

    if (temp && strcmp(temp->city, site->city) == 0)
        return 0;

    site->next = temp;
    if (prev)
        prev->next = site;
    else
        hotel->first = site;
    return 1;
}

int deleteSite(T_Hotel *hotel, char *city)
{
    if (!hotel || !city)
        return 0;

    T_Site *prev = NULL;
    T_Site *temp = hotel->first;
    while (temp && strcmp(temp->city, city) != 0) {
        prev = temp;
        temp = temp->next;
    }
    if (!temp)
        return 0;

    if (prev)
        prev->next = temp->next;
    else
        hotel->first = temp->next;

    freeClients(temp->first);
    free(temp->city);
    free(temp);
    return 1;
}

T_Site *findSite(T_Hotel *hotel, char *city)
{
    if (!hotel || !city)
        return NULL;

    T_Site *temp = hotel->first;
    while (temp && strcmp(temp->city, city) != 0)
        temp = temp->next;
    return temp;
}
