#pragma once
#include "tp3.h"

T_Result *findClients  (T_Hotel *hotel, char *critereaName);
void      searchClients(T_Hotel *hotel, char *critereaName);
void      freeResults  (T_Result *head);
