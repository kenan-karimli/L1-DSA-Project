#pragma once
#include "tp3.h"

T_Site *createSite(char *city);
int     addSite   (T_Hotel *hotel, T_Site *site);
int     deleteSite(T_Hotel *hotel, char *city);
T_Site *findSite  (T_Hotel *hotel, char *city);
