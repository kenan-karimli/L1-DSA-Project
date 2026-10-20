#include "client.h"

char *copyString(const char *s)
{
    if (!s)
        return NULL;
    char *copy = malloc(strlen(s) + 1);
    if (copy)
        strcpy(copy, s);
    return copy;
}

/* < 0 if (last1, first1) comes before (last2, first2), 0 if equal, > 0 after */
int compareNames(const char *last1, const char *first1, const char *last2, const char *first2)
{
    int cmp = strcmp(last1, last2);
    if (cmp != 0)
        return cmp;
    return strcmp(first1, first2);
}

T_Client *createClient(char *last_name, char *first_name, int age, int num_room)
{
    if (!last_name || !first_name)
        return NULL;

    T_Client *newClient = malloc(sizeof(T_Client));
    if (!newClient)
        return NULL;

    newClient->last_name  = copyString(last_name);
    newClient->first_name = copyString(first_name);
    if (!newClient->last_name || !newClient->first_name) {
        free(newClient->last_name);
        free(newClient->first_name);
        free(newClient);
        return NULL;
    }
    newClient->age      = age;
    newClient->num_room = num_room;
    newClient->next     = NULL;
    return newClient;
}

/* Sorted insert by last name, then first name. A client with the same last and
 * first name is rejected, so deleteClient always knows which one to remove.
 * On failure the caller still owns the client and must free it. */
int addClient(T_Site *site, T_Client *client)
{
    if (!site || !client)
        return 0;

    T_Client *prev = NULL;
    T_Client *temp = site->first;
    while (temp && compareNames(temp->last_name, temp->first_name,
                                client->last_name, client->first_name) < 0) {
        prev = temp;
        temp = temp->next;
    }

    if (temp && compareNames(temp->last_name, temp->first_name,
                             client->last_name, client->first_name) == 0)
        return 0;

    client->next = temp;
    if (prev)
        prev->next = client;
    else
        site->first = client;

    site->number_clients++;
    return 1;
}

int deleteClient(T_Site *site, char *last_name, char *first_name)
{
    if (!site || !last_name || !first_name)
        return 0;

    T_Client *prev = NULL;
    T_Client *temp = site->first;
    while (temp && compareNames(temp->last_name, temp->first_name, last_name, first_name) != 0) {
        prev = temp;
        temp = temp->next;
    }
    if (!temp)
        return 0;

    if (prev)
        prev->next = temp->next;
    else
        site->first = temp->next;

    free(temp->last_name);
    free(temp->first_name);
    free(temp);
    site->number_clients--;
    return 1;
}

void freeClients(T_Client *head)
{
    T_Client *temp;
    while (head) {
        temp = head;
        head = head->next;
        free(temp->last_name);
        free(temp->first_name);
        free(temp);
    }
}
