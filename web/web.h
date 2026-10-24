#ifndef WEB_H
#define WEB_H

#include "tp3.h"

/* Extra C functions used only by the Python GUIs (not part of the submission). */

#define SAVE_FILE "data/saved_chain.txt"   /* must match SAVE_FILE in hotel_lib.py */

void loadBookings(T_Hotel *hotel, char *filename);
int  saveHotel(T_Hotel *hotel, char *filename);
int  saveChainFile(T_Hotel *hotel, char *filename, char *passHash);

#endif
