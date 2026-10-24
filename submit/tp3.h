#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME    50
#define BOOKINGS_FILE "data/bookings.txt"   /* processingfile has no filename parameter */


typedef struct Client{
    char *last_name;
    char *first_name;
    int  age;
    int num_room;
    struct Client *next;
}T_Client;

typedef struct Site {
    char *city;
    int number_clients;
    T_Client *first;
    struct Site *next;
} T_Site;

typedef struct Hotel {
    char *name;
    T_Site *first;
}T_Hotel;



/* One search result: points to an existing client, nothing is copied. */
typedef struct Result {
    T_Client  *client;
    char *city;
    struct Result *next;
} T_Result;



T_Client *createClient(char *last_name,char *first_name,int age,int num_room);
T_Site *createSite(char *city);
T_Hotel *createHotel(char *name);

int  addClient(T_Site *site, T_Client *client);
int  addSite(T_Hotel *hotel,T_Site *site);
void displayHotel(T_Hotel *hotel);
void displaySite(T_Site *site);
int  deleteClient(T_Site *site, char *last_name,char *first_name);
int  deleteSite(T_Hotel *hotel,char *city);
void searchClients(T_Hotel *hotel, char *critereaName);
void processingfile(T_Hotel *hotel);


char *copyString(char *s);
int compareNames(char *last1, char *first1,char *last2, char *first2);
T_Site *findSite(T_Hotel *hotel,char *city);
T_Result *findClients(T_Hotel *hotel,char *critereaName);
int addBooking(T_Hotel *hotel,char *last_name, char *first_name,int age, int num_room, char *city);
void freeResults(T_Result *head);
void freeClients(T_Client *head);
void freeHotel(T_Hotel *hotel);

