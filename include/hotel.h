#pragma once
#include "tp3.h"

T_Hotel *createHotel (char *name);
void     displayHotel(T_Hotel *hotel);
void     displaySite (T_Site *site);
void     freeHotel   (T_Hotel *hotel);
void     printCell   (const char *text, int width);
