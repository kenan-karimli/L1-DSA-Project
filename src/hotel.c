#include "hotel.h"
#include "client.h"

T_Hotel *createHotel(char *name)
{
    if (!name)
        return NULL;

    T_Hotel *newHotel = malloc(sizeof(T_Hotel));
    if (!newHotel)
        return NULL;

    newHotel->name = copyString(name);
    if (!newHotel->name) {
        free(newHotel);
        return NULL;
    }
    newHotel->first = NULL;
    return newHotel;
}

/* Prints "| text" padded to width characters. printf pads by bytes,
 * so accented letters (2 bytes in UTF-8) would shift the table border. */
void printCell(const char *text, int width)
{
    int chars = 0;
    for (const char *p = text; *p; p++)
        if (((unsigned char)*p & 0xC0) != 0x80)   /* skip UTF-8 continuation bytes */
            chars++;
    printf("| %s%*s ", text, width > chars ? width - chars : 0, "");
}

void displayHotel(T_Hotel *hotel)
{
    if (!hotel) {
        printf("No hotel chain.\n");
        return;
    }

    printf("\nHotel chain: %s\n", hotel->name);
    if (!hotel->first) {
        printf("No locations yet.\n");
        return;
    }

    printf("+----------------------+----------------------+\n");
    printf("| %-20s | %-20s |\n", "City", "Number of customers");
    printf("+----------------------+----------------------+\n");
    for (T_Site *temp = hotel->first; temp; temp = temp->next) {
        printCell(temp->city, 20);
        printf("| %-20d |\n", temp->number_clients);
    }
    printf("+----------------------+----------------------+\n");
}

void displaySite(T_Site *site)
{
    if (!site) {
        printf("No such location.\n");
        return;
    }

    printf("\nLocation: %s (%d customers)\n", site->city, site->number_clients);
    if (!site->first) {
        printf("No customers yet.\n");
        return;
    }

    printf("+----------------------+----------------------+-------+-------+\n");
    printf("| %-20s | %-20s | %-5s | %-5s |\n", "Last name", "First name", "Age", "Room");
    printf("+----------------------+----------------------+-------+-------+\n");
    for (T_Client *temp = site->first; temp; temp = temp->next) {
        printCell(temp->last_name, 20);
        printCell(temp->first_name, 20);
        printf("| %-5d | %-5d |\n", temp->age, temp->num_room);
    }
    printf("+----------------------+----------------------+-------+-------+\n");
}

void freeHotel(T_Hotel *hotel)
{
    if (!hotel)
        return;

    T_Site *temp;
    while (hotel->first) {
        temp         = hotel->first;
        hotel->first = temp->next;
        freeClients(temp->first);
        free(temp->city);
        free(temp);
    }
    free(hotel->name);
    free(hotel);
}
