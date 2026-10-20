#include <stdio.h>
#include "tp3.h"
#include "client.h"
#include "site.h"
#include "hotel.h"
#include "search.h"
#include "file.h"

/*
 * DO NOT DELETE: the program's only main() lives here.
 * Never write main() in any other src/ file: the build would fail with "multiple definition of main".
 */

/* Reads one line into buf without the newline. Returns 0 on end of input. */
static int main_readLine(const char *prompt, char *buf, int size)
{
    printf("%s", prompt);
    fflush(stdout);
    if (!fgets(buf, size, stdin))
        return 0;

    char *newline = strchr(buf, '\n');
    if (newline) {
        *newline = '\0';
    } else {
        int c;  /* line was longer than buf: drop the rest */
        while ((c = getchar()) != '\n' && c != EOF)
            ;
    }
    return 1;
}

/* Asks until the user types a non-empty line. Returns 0 on end of input. */
static int main_readText(const char *prompt, char *buf, int size)
{
    while (main_readLine(prompt, buf, size)) {
        if (buf[0] != '\0')
            return 1;
        printf("Please type something.\n");
    }
    return 0;
}

/* Asks until the user types a whole number in [min, max]. Returns 0 on end of input. */
static int main_readInt(const char *prompt, int min, int max, int *value)
{
    char buf[32];
    char extra;
    while (main_readLine(prompt, buf, sizeof buf)) {
        if (sscanf(buf, "%d %c", value, &extra) == 1 && *value >= min && *value <= max)
            return 1;
        printf("Please type a number between %d and %d.\n", min, max);
    }
    return 0;
}

static int main_needHotel(T_Hotel *hotel)
{
    if (!hotel)
        printf("Create a hotel chain first (option 1).\n");
    return hotel != NULL;
}

static void main_createHotel(T_Hotel **hotel)
{
    char name[MAX_NAME];
    if (!main_readText("Hotel chain name: ", name, sizeof name))
        return;

    T_Hotel *newHotel = createHotel(name);
    if (!newHotel) {
        printf("Out of memory.\n");
        return;
    }
    if (*hotel)
        printf("The previous chain %s and all its data were deleted.\n", (*hotel)->name);
    freeHotel(*hotel);
    *hotel = newHotel;
    printf("Hotel chain %s created.\n", newHotel->name);
}

static void main_addSite(T_Hotel *hotel)
{
    char city[MAX_NAME];
    if (!main_needHotel(hotel) || !main_readText("City: ", city, sizeof city))
        return;

    T_Site *site = createSite(city);
    if (!site) {
        printf("Out of memory.\n");
        return;
    }
    if (addSite(hotel, site)) {
        printf("Location %s added.\n", city);
    } else {
        printf("Location %s already exists.\n", city);
        free(site->city);
        free(site);
    }
}

static void main_addClient(T_Hotel *hotel)
{
    char city[MAX_NAME], last_name[MAX_NAME], first_name[MAX_NAME];
    int  age, num_room;
    if (!main_needHotel(hotel) || !main_readText("City: ", city, sizeof city))
        return;

    T_Site *site = findSite(hotel, city);
    if (!site) {
        printf("No location in %s. Add it first (option 2).\n", city);
        return;
    }
    if (!main_readText("Last name: ", last_name, sizeof last_name)
        || !main_readText("First name: ", first_name, sizeof first_name)
        || !main_readInt("Age: ", 0, 150, &age)
        || !main_readInt("Room number: ", 1, 99999, &num_room))
        return;

    T_Client *client = createClient(last_name, first_name, age, num_room);
    if (!client) {
        printf("Out of memory.\n");
        return;
    }
    if (addClient(site, client)) {
        printf("Booking for %s %s added in %s.\n", last_name, first_name, city);
    } else {
        printf("%s %s already has a booking in %s.\n", last_name, first_name, city);
        freeClients(client);
    }
}

static void main_displaySite(T_Hotel *hotel)
{
    char city[MAX_NAME];
    if (!main_needHotel(hotel) || !main_readText("City: ", city, sizeof city))
        return;

    T_Site *site = findSite(hotel, city);
    if (site)
        displaySite(site);
    else
        printf("No location in %s.\n", city);
}

static void main_deleteClient(T_Hotel *hotel)
{
    char city[MAX_NAME], last_name[MAX_NAME], first_name[MAX_NAME];
    if (!main_needHotel(hotel) || !main_readText("City: ", city, sizeof city))
        return;

    T_Site *site = findSite(hotel, city);
    if (!site) {
        printf("No location in %s.\n", city);
        return;
    }
    if (!main_readText("Last name: ", last_name, sizeof last_name)
        || !main_readText("First name: ", first_name, sizeof first_name))
        return;

    if (deleteClient(site, last_name, first_name))
        printf("Booking for %s %s deleted.\n", last_name, first_name);
    else
        printf("No booking for %s %s in %s.\n", last_name, first_name, city);
}

static void main_deleteSite(T_Hotel *hotel)
{
    char city[MAX_NAME];
    if (!main_needHotel(hotel) || !main_readText("City: ", city, sizeof city))
        return;

    if (deleteSite(hotel, city))
        printf("Location %s and all its bookings deleted.\n", city);
    else
        printf("No location in %s.\n", city);
}

static void main_searchClients(T_Hotel *hotel)
{
    char prefix[MAX_NAME];
    if (!main_needHotel(hotel) || !main_readText("Last name starts with: ", prefix, sizeof prefix))
        return;
    searchClients(hotel, prefix);
}

static void main_printMenu(T_Hotel *hotel)
{
    printf("\n===== Hotel chain: %s =====\n", hotel ? hotel->name : "(none)");
    printf(" 1. Create a hotel chain\n");
    printf(" 2. Add a location to a hotel chain\n");
    printf(" 3. Add a booking at a location\n");
    printf(" 4. Display the locations in a hotel chain\n");
    printf(" 5. Display the customers at a location\n");
    printf(" 6. Delete a booking at a location\n");
    printf(" 7. Delete a location\n");
    printf(" 8. Search for a customer by last name\n");
    printf(" 9. Quit\n");
    printf("10. Load bookings from %s\n", BOOKINGS_FILE);
}

int main(void)
{
    T_Hotel *hotel = NULL;
    int      choice;

    do {
        main_printMenu(hotel);
        if (!main_readInt("Your choice: ", 1, 10, &choice))
            break;

        switch (choice) {
        case 1:  main_createHotel(&hotel);   break;
        case 2:  main_addSite(hotel);        break;
        case 3:  main_addClient(hotel);      break;
        case 4:  if (main_needHotel(hotel)) displayHotel(hotel); break;
        case 5:  main_displaySite(hotel);    break;
        case 6:  main_deleteClient(hotel);   break;
        case 7:  main_deleteSite(hotel);     break;
        case 8:  main_searchClients(hotel);  break;
        case 10: if (main_needHotel(hotel)) processingfile(hotel); break;
        }
    } while (choice != 9);

    freeHotel(hotel);
    printf("Goodbye.\n");
    return 0;
}
