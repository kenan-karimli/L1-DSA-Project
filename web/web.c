#include "web.h"

/* Like processingfile but for any file. "#SITE;city" lines create an
   (empty) site, other lines starting with '#' are ignored. */
void loadBookings(T_Hotel *hotel, char *filename)
{
    if (!hotel || !filename)
        return;

    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Cannot open %s.\n", filename);
        return;
    }

    char line[512], last_name[MAX_NAME], first_name[MAX_NAME], city[MAX_NAME];
    int  age, num_room, added = 0;
    while (fgets(line, sizeof line, file)) {
        if (sscanf(line, "#SITE;%49[^\r\n]", city) == 1) {
            if (!findSite(hotel, city))
                addSite(hotel, createSite(city));   /* addSite does nothing if createSite failed */
        } else if (line[0] != '#'
                   && sscanf(line, "%49[^;];%49[^;];%d;%d;%49[^\r\n]", last_name, first_name, &age, &num_room, city) == 5
                   && addBooking(hotel, last_name, first_name, age, num_room, city)) {
            added++;
        }
    }

    fclose(file);
    printf("%d bookings added.\n", added);
}

/* Writes the header line and every booking. Returns how many bookings. */
static int writeBookings(FILE *file, T_Hotel *hotel)
{
    int written = 0;
    fprintf(file, "LastName;FirstName;Age;Room Number;City\n");
    for (T_Site *site = hotel->first; site; site = site->next)
        for (T_Client *client = site->first; client; client = client->next, written++)
            fprintf(file, "%s;%s;%d;%d;%s\n", client->last_name, client->first_name,
                    client->age, client->num_room, site->city);
    return written;
}

/* Saves all bookings in the bookings.txt format.
   Returns how many bookings were written, or -1 on error. */
int saveHotel(T_Hotel *hotel, char *filename)
{
    if (!hotel || !filename)
        return -1;

    FILE *file = fopen(filename, "w");
    if (!file)
        return -1;
    int written = writeBookings(file, hotel);
    return fclose(file) == 0 ? written : -1;
}

/* Same as saveHotel, but first writes "#HOTEL;name;passHash" and one
   "#SITE;city" line per site so empty sites are kept. passHash can be "". */
int saveChainFile(T_Hotel *hotel, char *filename, char *passHash)
{
    if (!hotel || !filename || !passHash)
        return -1;

    FILE *file = fopen(filename, "w");
    if (!file)
        return -1;
    fprintf(file, "#HOTEL;%s;%s\n", hotel->name, passHash);
    for (T_Site *site = hotel->first; site; site = site->next)
        fprintf(file, "#SITE;%s\n", site->city);
    int written = writeBookings(file, hotel);
    return fclose(file) == 0 ? written : -1;
}
