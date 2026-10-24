#include "tp3.h"

/* Reads one line into buf without the '\n'. Returns 0 on end of input. */
static int readLine(char *prompt, char *buf,int size){
    printf("%s", prompt);
    if (!fgets(buf, size, stdin))
        return 0;

    char *newline = strchr(buf, '\n');
    if (newline) *newline = '\0';
    else {
        int c;   /* line too long: throw away the rest */
        while ((c = getchar()) != '\n' && c != EOF);
    }
    return 1;
}

/* Asks again until the user types a non-empty text. */
static int readText(char *prompt,char *buf, int size)
{
    while(readLine(prompt, buf,size)) {
        if(buf[0] != '\0')
            return 1;
        printf("Please type something.\n");
    }
    return 0;
}

/* Asks again until the user types a number between min and max. */
static int readInt(char *prompt,int min, int max, int *value){
    char buf[32], extra;
    while(readLine(prompt, buf, sizeof buf)){

        /* "%c" catches extra text like "12abc" */
        if(sscanf(buf, "%d %c",value, &extra) ==1 && *value >= min && *value <= max) return 1;
        printf("Please type a number between %d and %d.\n", min, max);
    }
    return 0;
}

/* Prints a message and returns 0 if no hotel chain exists yet. */
static int needHotel(T_Hotel *hotel){
    if(!hotel) printf("Create a hotel chain first (option 1).\n");
    return hotel !=NULL;
}

/* Asks for a city and returns its site, or NULL if it does not exist. */
static T_Site *askSite(T_Hotel *hotel){
    char city[MAX_NAME];
    if(!readText("City: ",city, sizeof city))
        return NULL;

    T_Site *site = findSite(hotel, city);
    if (!site)
        printf("No location in %s.\n", city);
    return site;
}

/* Option 1: replaces the current chain with a new empty one. */
static void menuCreateHotel(T_Hotel **hotel){
    char name[MAX_NAME];
    if (!readText("Hotel chain name: ", name,sizeof name))
        return;

    T_Hotel *newHotel = createHotel(name);
    if (!newHotel) {
        printf("Out of memory.\n");
        return;
    }
    freeHotel(*hotel);
    *hotel = newHotel;
    printf("Hotel chain %s created.\n", name);
}

/* Option 2 */
static void menuAddSite(T_Hotel *hotel){
    char city[MAX_NAME];
    if(!readText("City: ",city, sizeof city))
        return;

    T_Site *site = createSite(city);
    if (!site) printf("Out of memory.\n");
    else if (addSite(hotel, site)) printf("Location %s added.\n", city);
    else{
        printf("Location %s already exists.\n", city);
        free(site->city);
        free(site);
    }
}

/* Option 3 */
static void menuAddClient(T_Hotel *hotel)
{
    char last_name[MAX_NAME], first_name[MAX_NAME];
    int  age,num_room;
    T_Site *site = askSite(hotel);
    if(!site || !readText("Last name: ",last_name,sizeof last_name) || !readText("First name: ",first_name,sizeof first_name) || !readInt("Age: ",0, 150, &age) || !readInt("Room number: ", 1, 99999, &num_room))
        return;

    T_Client *client = createClient(last_name, first_name, age, num_room);
    if (!client) printf("Out of memory.\n");
    else if(addClient(site, client)) printf("Booking added.\n");
    else{
        printf("%s %s already has a booking here.\n", last_name, first_name);
        freeClients(client);
    }
}

/* Option 6 */
static void menuDeleteClient(T_Hotel *hotel)
{
    char last_name[MAX_NAME], first_name[MAX_NAME];
    T_Site *site = askSite(hotel);
    if (!site || !readText("Last name: ", last_name, sizeof last_name) || !readText("First name: ", first_name, sizeof first_name))
        return;

    if(deleteClient(site, last_name, first_name))
        printf("Booking deleted.\n");
    else
        printf("No booking for %s %s.\n", last_name, first_name);
}

/* Option 7 */
static void menuDeleteSite(T_Hotel *hotel){
    char city[MAX_NAME];
    if (!readText("City: ", city, sizeof city))
        return;

    if (deleteSite(hotel, city))
        printf("Location %s deleted.\n", city);
    else
        printf("No location in %s.\n", city);
}

/* Option 8 */
static void menuSearch(T_Hotel *hotel){
    char prefix[MAX_NAME];
    if (readText("Last name starts with: ", prefix, sizeof prefix))
        searchClients(hotel, prefix);
}

static void printMenu(void){
    printf("\n1. Create a hotel chain\n");
    printf("2. Add a location to a hotel chain\n");
    printf("3. Add a booking at a location\n");
    printf("4. Display the locations in a hotel chain\n");
    printf("5. Display the customers at a location\n");
    printf("6. Delete a booking at a location\n");
    printf("7. Delete a location\n");
    printf("8. Search for a customer by last name\n");
    printf("9. Quit\n");
    printf("10. Load the bookings file (%s)\n", BOOKINGS_FILE);
}

int main(void)
{
    T_Hotel *hotel = NULL;
    int choice = 0;

    while (choice != 9) {
        printMenu();
        if(!readInt("Your choice: ", 1, 10, &choice))
            break;
        if (choice == 1) {
            menuCreateHotel(&hotel);
            continue;
        }
        if (choice == 9 || !needHotel(hotel))
            continue;

        switch (choice){
            case 2:  menuAddSite(hotel);
                break;
            case 3:  menuAddClient(hotel);
                break;
            case 4:  displayHotel(hotel);
                break;
            case 5:  displaySite(askSite(hotel));
                break;
            case 6:  menuDeleteClient(hotel);
                break;
            case 7:  menuDeleteSite(hotel);
                break;
            case 8:  menuSearch(hotel);
                break;
            case 10: processingfile(hotel);
                break;
        }
    }

    freeHotel(hotel);
    printf("Goodbye.\n");
    return 0;
}
