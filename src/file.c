#include "file.h"
#include "client.h"
#include "site.h"

/* a line looks like: LastName;FirstName;Age;Room Number;City
   returns 1 if we got all 5 fields */
static int file_parseLine(const char *line, char *last_name, char *first_name,
                          int *age, int *num_room, char *city)
{
    /* 49 = MAX_NAME - 1 (need space for the \0) */
    return sscanf(line, " %49[^;];%49[^;];%d;%d;%49[^\r\n]",
                  last_name, first_name, age, num_room, city) == 5;
}

/* finds the site for city, or creates it if its not there yet. NULL if malloc failed */
static T_Site *file_getSite(T_Hotel *hotel, const char *city)
{
    T_Site *site = findSite(hotel, (char *)city);
    if (site)
        return site;
    site = createSite((char *)city);
    if (!site || !addSite(hotel, site)) {
        free(site ? site->city : NULL);
        free(site);
        return NULL;
    }
    return site;
}

void processingfile(T_Hotel *hotel)
{
    loadBookings(hotel, BOOKINGS_FILE);
}

/* reads the file and adds every booking to the hotel.
   bad lines are skipped and we print which line it was */
void loadBookings(T_Hotel *hotel, const char *filename)
{
    if (!hotel) {
        printf("Create a hotel chain first.\n");
        return;
    }

    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Cannot open %s.\n", filename);
        return;
    }

    char line[512];   /* the #HOTEL line can be ~220 chars with the hash */
    char last_name[MAX_NAME], first_name[MAX_NAME], city[MAX_NAME];
    int  age, num_room;
    int  added = 0, skipped = 0, lineNumber = 0;

    while (fgets(line, sizeof line, file)) {
        lineNumber++;
        if (strncmp(line, "LastName;", 9) == 0)
            continue;
        if (line[0] == '\n' || line[0] == '\r')
            continue;

        /* lines starting with # are not bookings.
           #SITE;city makes an (maybe empty) location, #HOTEL is read by python, others are ignored */
        if (line[0] == '#') {
            if (strncmp(line, "#SITE;", 6) == 0) {
                if (sscanf(line, "#SITE;%49[^\r\n]", city) != 1 || !file_getSite(hotel, city)) {
                    printf("Line %d skipped: bad #SITE line.\n", lineNumber);
                    skipped++;
                }
            }
            continue;
        }

        if (!file_parseLine(line, last_name, first_name, &age, &num_room, city)) {
            printf("Line %d skipped: wrong format.\n", lineNumber);
            skipped++;
            continue;
        }

        T_Site *site = file_getSite(hotel, city);
        if (!site) {
            printf("Line %d skipped: cannot create location %s.\n", lineNumber, city);
            skipped++;
            continue;
        }

        T_Client *client = createClient(last_name, first_name, age, num_room);
        if (!client) {
            printf("Line %d skipped: out of memory.\n", lineNumber);
            skipped++;
            continue;
        }
        if (!addClient(site, client)) {
            freeClients(client);
            printf("Line %d skipped: %s %s is already booked in %s.\n",
                   lineNumber, last_name, first_name, city);
            skipped++;
            continue;
        }
        added++;
    }

    fclose(file);
    printf("%d bookings added, %d lines skipped.\n", added, skipped);
}

/* writes the header + all bookings, same for both save functions. returns how many bookings */
static int file_writeBookings(FILE *file, T_Hotel *hotel)
{
    int written = 0;
    fprintf(file, "LastName;FirstName;Age;Room Number;City\n");
    for (T_Site *site = hotel->first; site; site = site->next)
        for (T_Client *client = site->first; client; client = client->next, written++)
            fprintf(file, "%s;%s;%d;%d;%s\n", client->last_name, client->first_name,
                    client->age, client->num_room, site->city);
    return written;
}

/* saves all bookings in the same format as bookings.txt so loadBookings can read it again.
 * NOTE: chain name and empty locations are not saved here, use saveChainFile for that
 * returns how many bookings were written, or -1 if something went wrong */
int saveHotel(T_Hotel *hotel, const char *filename)
{
    if (!hotel || !filename)
        return -1;

    FILE *file = fopen(filename, "w");
    if (!file)
        return -1;

    int written = file_writeBookings(file, hotel);

    /* fclose can fail too (if the data didnt get written fully) so check it */
    if (fclose(file) != 0)
        return -1;
    return written;
}

/* same as saveHotel but for the web app: first a #HOTEL;name;hash line,
 * then #SITE;city for every site (so empty ones are kept too), then the bookings.
 * hash is the file password hash made in python, "" if the file has no password */
int saveChainFile(T_Hotel *hotel, const char *filename, const char *passHash)
{
    if (!hotel || !filename || !passHash)
        return -1;

    FILE *file = fopen(filename, "w");
    if (!file)
        return -1;

    fprintf(file, "#HOTEL;%s;%s\n", hotel->name, passHash);
    for (T_Site *site = hotel->first; site; site = site->next)
        fprintf(file, "#SITE;%s\n", site->city);
    int written = file_writeBookings(file, hotel);

    if (fclose(file) != 0)
        return -1;
    return written;
}
