#include <ctype.h>
#include "search.h"
#include "client.h"
#include "hotel.h"

/* Case-insensitive, so "dup" finds DUPOND and Dupont */
static int search_startsWith(const char *name, const char *prefix)
{
    while (*prefix) {
        if (toupper((unsigned char)*name) != toupper((unsigned char)*prefix))
            return 0;
        name++;
        prefix++;
    }
    return 1;
}

/* Sorted insert by last name, first name, then city */
static int search_insertResult(T_Result **head, T_Client *client, char *city)
{
    T_Result *newResult = malloc(sizeof(T_Result));
    if (!newResult)
        return 0;
    newResult->client = client;
    newResult->city   = city;

    T_Result *prev = NULL;
    T_Result *temp = *head;
    while (temp) {
        int cmp = compareNames(temp->client->last_name, temp->client->first_name,
                               client->last_name, client->first_name);
        if (cmp > 0 || (cmp == 0 && strcmp(temp->city, city) > 0))
            break;
        prev = temp;
        temp = temp->next;
    }

    newResult->next = temp;
    if (prev)
        prev->next = newResult;
    else
        *head = newResult;
    return 1;
}

/* Every client whose last name starts with critereaName, across all sites, sorted.
 * The result points into the hotel's nodes; free it with freeResults only. */
T_Result *findClients(T_Hotel *hotel, char *critereaName)
{
    T_Result *results = NULL;
    if (!hotel || !critereaName)
        return NULL;

    for (T_Site *site = hotel->first; site; site = site->next)
        for (T_Client *client = site->first; client; client = client->next)
            if (search_startsWith(client->last_name, critereaName)
                && !search_insertResult(&results, client, site->city)) {
                freeResults(results);
                return NULL;
            }
    return results;
}

void searchClients(T_Hotel *hotel, char *critereaName)
{
    T_Result *results = findClients(hotel, critereaName);
    if (!results) {
        printf("No customer whose last name starts with \"%s\".\n", critereaName ? critereaName : "");
        return;
    }

    printf("+----------------------+----------------------+-------+-------+----------------------+\n");
    printf("| %-20s | %-20s | %-5s | %-5s | %-20s |\n", "Last name", "First name", "Age", "Room", "City");
    printf("+----------------------+----------------------+-------+-------+----------------------+\n");
    for (T_Result *temp = results; temp; temp = temp->next) {
        printCell(temp->client->last_name, 20);
        printCell(temp->client->first_name, 20);
        printf("| %-5d | %-5d ", temp->client->age, temp->client->num_room);
        printCell(temp->city, 20);
        printf("|\n");
    }
    printf("+----------------------+----------------------+-------+-------+----------------------+\n");

    freeResults(results);
}

/* Frees only the result nodes, never the clients or cities they point to */
void freeResults(T_Result *head)
{
    T_Result *temp;
    while (head) {
        temp = head;
        head = head->next;
        free(temp);
    }
}
