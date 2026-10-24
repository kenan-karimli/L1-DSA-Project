#include <ctype.h>
#include "tp3.h"


/* Returns a malloc'd copy of s, or NULL if malloc fails. */
char *copyString(char *s){
    char *copy = malloc(strlen(s) + 1);
    if (copy)
        strcpy(copy, s);
    return copy;
}

/* Compares two people by last name, then first name (like strcmp). */
int compareNames(char *last1,char *first1,char *last2, char *first2){
    int cmp =strcmp(last1, last2);
    if (cmp != 0) return cmp;
    return strcmp(first1, first2);
}

/* Returns the site of the given city, or NULL if there is none. */
T_Site *findSite(T_Hotel *hotel, char *city){
    if (!hotel || !city)
        return NULL;
    T_Site *temp = hotel->first;
    while (temp && strcmp(temp->city, city) != 0)
        temp = temp->next;
    return temp;
}



/* Creates a client with copies of the names. Returns NULL on error. */
T_Client *createClient(char *last_name, char *first_name,int age, int num_room){
    if (!last_name || !first_name)
        return NULL;
    T_Client *client = malloc(sizeof(T_Client));
    if(!client)
        return NULL;

    client->last_name = copyString(last_name);
    client->first_name = copyString(first_name);
    if (!client->last_name || !client->first_name){
        free(client->last_name);
        free(client->first_name);
        free(client);
        return NULL;
    }
    client->age = age;
    client->num_room = num_room;
    client->next = NULL;
    return client;
}

/* Creates an empty site for the given city. Returns NULL on error. */
T_Site *createSite(char *city){
    if (!city)
        return NULL;

    T_Site *site = malloc(sizeof(T_Site));
    if(!site)
        return NULL;

    site->city = copyString(city);
    if (!site->city){
        free(site);
        return NULL;
    }
    site->number_clients = 0;
    site->first = NULL;
    site->next = NULL;
    return site;
}

/* Creates an empty hotel chain. Returns NULL on error. */
T_Hotel *createHotel(char *name){
    if(!name)
        return NULL;
    T_Hotel *hotel = malloc(sizeof(T_Hotel));
    if (!hotel)
        return NULL;

    hotel->name =copyString(name);
    if (!hotel->name) {
        free(hotel);
        return NULL;
    }
    hotel->first = NULL;
    return hotel;
}


/* Inserts the client sorted by last name, then first name.
   Returns 0 if the same person is already there (the caller frees the client). */
int addClient(T_Site *site, T_Client *client){
    if(!site || !client)
        return 0;
    T_Client *prev = NULL;
    T_Client *temp =site->first;
    while (temp && compareNames(temp->last_name, temp->first_name,client->last_name, client->first_name) < 0) {
        prev = temp;
        temp = temp->next;
    }

    if (temp && compareNames(temp->last_name,temp->first_name,client->last_name, client->first_name) == 0)
        return 0;
    client->next = temp;
    if(prev) prev->next = client;
    else site->first = client;
    site->number_clients++;
    return 1;
}

/* Inserts the site sorted by city name.
   Returns 0 if the city already exists (the caller frees the site). */
int addSite(T_Hotel *hotel, T_Site *site){
    if(!hotel || !site)
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
    if (prev) prev->next = site;
    else hotel->first = site;
    return 1;
}


/* Prints all sites (already sorted by city) with their number of clients. */
void displayHotel(T_Hotel *hotel){
    if (!hotel)
        return;
    printf("\nHotel chain: %s\n", hotel->name);
    printf("%-20s %s\n", "City", "Number of customers");
    for(T_Site *temp = hotel->first; temp; temp = temp->next)
        printf("%-20s %d\n", temp->city, temp->number_clients);
}

/* Prints all clients of a site (already sorted by name). */
void displaySite(T_Site *site)
{
    if(!site) return;

    printf("\nLocation: %s\n", site->city);
    printf("%-20s %-20s %-6s %s\n", "Last name", "First name", "Age", "Room");
    for (T_Client *temp = site->first; temp; temp = temp->next)
        printf("%-20s %-20s %-6d %d\n", temp->last_name, temp->first_name, temp->age, temp->num_room);
}


/* Removes and frees one client. Returns 1 if found, 0 otherwise. */
int deleteClient(T_Site *site, char *last_name, char *first_name){
    if (!site || !last_name || !first_name) return 0;

    T_Client *prev = NULL;
    T_Client *temp = site->first;
    while (temp && compareNames(temp->last_name, temp->first_name, last_name, first_name) != 0) {
        prev = temp;
        temp = temp->next;
    }

    if (!temp) return 0;
    if (prev) prev->next = temp->next;
    else site->first = temp->next;

    free(temp->last_name);
    free(temp->first_name);
    free(temp);
    site->number_clients--;
    return 1;
}

/* Removes a site and frees it with all its clients. Returns 1 if found, 0 otherwise. */
int deleteSite(T_Hotel *hotel, char *city){
    if(!hotel || !city)
        return 0;

    T_Site *prev = NULL;
    T_Site *temp = hotel->first;
    while (temp && strcmp(temp->city, city) != 0){
        prev = temp;
        temp = temp->next;
    }
    if (!temp) return 0;
    if (prev) prev->next = temp->next;
    else hotel->first = temp->next;

    freeClients(temp->first);
    free(temp->city);
    free(temp);
    return 1;
}


/* Returns 1 if name starts with prefix (not case sensitive). */
static int startsWith(char *name, char *prefix){
    while(*prefix) {
        if(toupper((unsigned char)*name) != toupper((unsigned char)*prefix))
            return 0;

        name++;
        prefix++;
    }
    return 1;
}

/* Inserts a result sorted by last name, first name, then city. Returns 0 if malloc fails. */
static int insertResult(T_Result **head, T_Client *client, char *city){
    T_Result *result = malloc(sizeof(T_Result));

    if (!result) return 0;
    result->client = client;
    result->city   = city;

    T_Result *prev = NULL;
    T_Result *temp = *head;
    while(temp){
        int cmp = compareNames(temp->client->last_name, temp->client->first_name,
                               client->last_name, client->first_name);
        if (cmp > 0 || (cmp == 0 && strcmp(temp->city, city) > 0))
            break;
        prev = temp;
        temp = temp->next;
    }

    result->next = temp;
    if(prev) prev->next =result;
    else *head = result;
    return 1;
}

/* Returns a sorted list of clients (from all sites) whose last name starts with critereaName.
   Free it with freeResults, not freeClients. */
T_Result *findClients(T_Hotel *hotel, char *critereaName){
    T_Result *results =NULL;

    if (!hotel || !critereaName) return NULL;

    for (T_Site *site = hotel->first; site; site = site->next)
        for (T_Client *client = site->first; client; client = client->next)
            if(startsWith(client->last_name, critereaName) && !insertResult(&results, client, site->city)){
                freeResults(results);
                return NULL;
            }
    return results;
}

/* Prints the clients whose last name starts with critereaName, sorted by name. */
void searchClients(T_Hotel *hotel,char *critereaName)
{
    T_Result *results =findClients(hotel, critereaName);
    if(!results) {
        printf("No customer found.\n");
        return;
    }

    printf("%-20s %-20s %-6s %-6s %s\n", "Last name", "First name", "Age", "Room", "City");
    for(T_Result *temp = results; temp; temp = temp->next)
        printf("%-20s %-20s %-6d %-6d %s\n", temp->client->last_name, temp->client->first_name, temp->client->age, temp->client->num_room, temp->city);
    freeResults(results);
}

/* Frees only the result nodes (the clients belong to the hotel). */
void freeResults(T_Result *head){
    while(head){
        T_Result *temp = head;
        head = head->next;
        free(temp);
    }
}


/* Adds one booking, creating the site if needed. Returns 1 on success, 0 otherwise. */
int addBooking(T_Hotel *hotel, char *last_name, char *first_name, int age, int num_room, char *city){
    if(!hotel) return 0;

    T_Site *site = findSite(hotel, city);
    if(!site){
        site = createSite(city);
        if (!site)
            return 0;
        addSite(hotel, site);   /* cannot fail, the city is not in the list */
    }

    T_Client *client = createClient(last_name, first_name, age, num_room);
    if (!client) return 0;
    if(!addClient(site, client)){
        freeClients(client);
        return 0;
    }
    return 1;
}

/* Reads BOOKINGS_FILE (LastName;FirstName;Age;Room Number;City) and adds every booking.
   The header line and bad lines are skipped because sscanf does not find 5 fields. */
void processingfile(T_Hotel *hotel){

    if (!hotel)return;
    FILE *file = fopen(BOOKINGS_FILE, "r");
    if (!file){
        printf("Cannot open %s.\n", BOOKINGS_FILE);
        return;
    }

    char line[256], last_name[MAX_NAME], first_name[MAX_NAME], city[MAX_NAME];
    int  age, num_room, added = 0;
    while(fgets(line, sizeof line, file))
        if (sscanf(line, "%49[^;];%49[^;];%d;%d;%49[^\r\n]", last_name, first_name, &age, &num_room, city) == 5
            && addBooking(hotel, last_name, first_name, age, num_room, city))
            added++;

    fclose(file);
    printf("%d bookings added.\n", added);
}



/* Frees a whole client list. */
void freeClients(T_Client *head){
    while (head){
        T_Client *temp = head;
        head = head->next;
        free(temp->last_name);
        free(temp->first_name);
        free(temp);
    }
}

/* Frees the hotel with all its sites and clients. */
void freeHotel(T_Hotel *hotel){
    if (!hotel) return;

    while (hotel->first)
        deleteSite(hotel, hotel->first->city);

    free(hotel->name);
    free(hotel);
}
