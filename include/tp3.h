#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME      50
#define BOOKINGS_FILE "data/bookings.txt"   /* processingfile has no filename param in the subject so we hardcode it */
#define SAVE_FILE     "data/saved_chain.txt"  /* saveHotel writes here (same format as bookings.txt) */

/* names are malloc'd with copyString, we free them when we free the node */
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

/* not in the subject, we added it for searchClients.
   it just points to clients that already exist, nothing is copied */
typedef struct Result {
    T_Client      *client;
    char          *city;
    struct Result *next;
} T_Result;
