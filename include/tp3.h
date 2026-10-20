#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME      50
#define BOOKINGS_FILE "data/bookings.txt"   /* processingfile takes no filename in the statement */

/* strings are heap-allocated with copyString and freed with their node */
typedef struct Client {
    char          *last_name;
    char          *first_name;
    int            age;
    int            num_room;
    struct Client *next;
} T_Client;

typedef struct Site {
    char        *city;
    int          number_clients;
    T_Client    *first;
    struct Site *next;
} T_Site;

typedef struct Hotel {
    char   *name;
    T_Site *first;
} T_Hotel;

/* our addition for searchClients: points into existing nodes, copies nothing */
typedef struct Result {
    T_Client      *client;
    char          *city;
    struct Result *next;
} T_Result;
