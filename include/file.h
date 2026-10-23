#pragma once
#include "tp3.h"

void processingfile(T_Hotel *hotel);
void loadBookings  (T_Hotel *hotel, const char *filename);
int  saveHotel     (T_Hotel *hotel, const char *filename);
int  saveChainFile (T_Hotel *hotel, const char *filename, const char *passHash);
