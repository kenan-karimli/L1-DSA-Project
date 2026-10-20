#include "file.h"
#include "client.h"
#include "site.h"

/* One line: LastName;FirstName;Age;Room Number;City. Returns 1 if all 5 fields were read. */
static int file_parseLine(const char *line, char *last_name, char *first_name,
                          int *age, int *num_room, char *city)
{
    /* widths are MAX_NAME - 1 */
    return sscanf(line, " %49[^;];%49[^;];%d;%d;%49[^\r\n]",
                  last_name, first_name, age, num_room, city) == 5;
}

void processingfile(T_Hotel *hotel)
{
    if (!hotel) {
        printf("Create a hotel chain first.\n");
        return;
    }

    FILE *file = fopen(BOOKINGS_FILE, "r");
    if (!file) {
        printf("Cannot open %s.\n", BOOKINGS_FILE);
        return;
    }

    char line[256];
    char last_name[MAX_NAME], first_name[MAX_NAME], city[MAX_NAME];
    int  age, num_room;
    int  added = 0, skipped = 0, lineNumber = 0;

    while (fgets(line, sizeof line, file)) {
        lineNumber++;
        if (lineNumber == 1 && strncmp(line, "LastName;", 9) == 0)
            continue;
        if (line[0] == '\n' || line[0] == '\r')
            continue;

        if (!file_parseLine(line, last_name, first_name, &age, &num_room, city)) {
            printf("Line %d skipped: wrong format.\n", lineNumber);
            skipped++;
            continue;
        }

        T_Site *site = findSite(hotel, city);
        if (!site) {
            site = createSite(city);
            if (!site || !addSite(hotel, site)) {
                free(site ? site->city : NULL);
                free(site);
                printf("Line %d skipped: cannot create location %s.\n", lineNumber, city);
                skipped++;
                continue;
            }
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
