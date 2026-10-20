#pragma once
#include "tp3.h"

char *copyString  (const char *s);
int   compareNames(const char *last1, const char *first1, const char *last2, const char *first2);

T_Client *createClient(char *last_name, char *first_name, int age, int num_room);
int       addClient   (T_Site *site, T_Client *client);
int       deleteClient(T_Site *site, char *last_name, char *first_name);
void      freeClients (T_Client *head);
