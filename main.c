#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUSES 100
#define MAX_ROUTES 50
#define MAX_STOPS 10
#define MAX_TICKETS 500

/* ===== DATA STRUCTURES ===== */
struct Bus {
    int busId;
    char regNo[20];
    int routeNo;
    char driver[30];
    int capacity;
    int available;
    int status;
};

struct Route {
    int routeNo;
    char name[40];
    int baseFare;
    int ratePerStop;
    int stopCount;
    char stops[MAX_STOPS][30];
};

struct Passenger {
    int ticketId;
    char name[30];
    int age;
    int busId;
    int routeNo;
    char fromName[30];
    char toName[30];
    int travelHour;
    int seats;
    int fare;
    int status;
};

struct Bus buses[MAX_BUSES];
int busCount = 0;
struct Route routes[MAX_ROUTES];
int routeCount = 0;
struct Passenger passengers[MAX_TICKETS];
int passengerCount = 0;

char statusNames[4][16] = {"", "Active", "Maintenance", "Not in service"};
char ticketStatusNames[3][12] = {"", "Booked", "Cancelled"};
char loadNames[3][8] = {"Low", "Medium", "High"};

/* ================= INPUT HELPERS ================= */
void readLine(char text[], int size) {
    if (fgets(text, size, stdin) == NULL) { exit(0); }
    int len = strlen(text);
    if (len > 0 && text[len - 1] == '\n') text[len - 1] = '\0';
    else while (getchar() != '\n');
}

void readNonEmpty(char prompt[], char text[], int size) {
    do {
        printf("%s", prompt);
        readLine(text, size);
    } while (strlen(text) == 0);
}

int readInt(char prompt[]) {
    char buffer[50];
    int value;
    while (1) {
        printf("%s", prompt);
        readLine(buffer, 50);
        if (sscanf(buffer, "%d", &value) == 1) return value;
        printf("  Invalid number.\n");
    }
}

int readIntInRange(char prompt[], int min, int max) {
    int value = readInt(prompt);
    while (value < min || value > max) {
        printf("  Invalid! Range: %d to %d.\n", min, max);
        value = readInt(prompt);
    }
    return value;
}

int askYesNo(char prompt[]) {
    char answer[10];
    printf("%s", prompt);
    readLine(answer, 10);
    return (answer[0] == 'y' || answer[0] == 'Y');
}

/* ================= DEFAULT DATA INITIALIZER ================= */
void initDefaults() {
    if (routeCount == 0) {
        routes[0] = (struct Route){101, "Erode to Salem", 20, 10, 4, {"Erode", "Sankari", "Elampillai", "Salem"}};
        routes[1] = (struct Route){102, "Chennai to Central", 15, 5, 3, {"Tambaram", "Guindy", "Central"}};
        routes[2] = (struct Route){103, "Coimbatore to Ooty", 30, 15, 3, {"Coimbatore", "Mettupalayam", "Ooty"}};
        routeCount = 3;
    }
    if (busCount == 0) {
        buses[0] = (struct Bus){1, "TN33AB1234", 101, "Ramesh", 40, 40, 1};
        buses[1] = (struct Bus){2, "TN33CD5678", 101, "Suresh", 50, 50, 1};
        buses[2] = (struct Bus){3, "TN01EF9012", 102, "Kumar", 35, 35, 1};
        buses[3] = (struct Bus){4, "TN37GH3456", 103, "Rajesh", 45, 45, 1};
        busCount = 4;
    }
}

/* ================= FILE LOAD/SAVE ================= */
void loadBuses() {
    FILE *fp = fopen("buses.dat", "rb");
    if (fp) {
        if (fread(&busCount, sizeof(int), 1, fp) == 1) fread(buses, sizeof(struct Bus), busCount, fp);
        fclose(fp);
    }
}

void saveBuses() {
    FILE *fp = fopen("buses.dat", "wb");
    if (fp) {
        fwrite(&busCount, sizeof(int), 1, fp);
        fwrite(buses, sizeof(struct Bus), busCount, fp);
        fclose(fp);
    }
}

void loadRoutes() {
    FILE *fp = fopen("routes.dat", "rb");
    if (fp) {
        if (fread(&routeCount, sizeof(int), 1, fp) == 1) fread(routes, sizeof(struct Route), routeCount, fp);
        fclose(fp);
    }
}

void saveRoutes() {
    FILE *fp = fopen("routes.dat", "wb");
    if (fp) {
        fwrite(&routeCount, sizeof(int), 1, fp);
        fwrite(routes, sizeof(struct Route), routeCount, fp);
        fclose(fp);
    }
}

void loadPassengers() {
    FILE *fp = fopen("passengers.dat", "rb");
    if (fp) {
        if (fread(&passengerCount, sizeof(int), 1, fp) == 1) fread(passengers, sizeof(struct Passenger), passengerCount, fp);
        fclose(fp);
    }
}

void savePassengers() {
    FILE *fp = fopen("passengers.dat", "wb");
    if (fp) {
        fwrite(&passengerCount, sizeof(int), 1, fp);
        fwrite(passengers, sizeof(struct Passenger), passengerCount, fp);
        fclose(fp);
    }
}

/* ================= LOOKUP HELPERS ================= */
int findBusIndex(int id) {
    for (int i = 0; i < busCount; i++) if (buses[i].busId == id) return i;
    return -1;
}

int findBusByReg(char reg[]) {
    for (int i = 0; i < busCount; i++) if (strcmp(buses[i].regNo, reg) == 0) return i;
    return -1;
}

int findRouteIndex(int routeNo) {
    for (int i = 0; i < routeCount; i++) if (routes[i].routeNo == routeNo) return i;
    return -1;
}

int findTicketIndex(int id) {
    for (int i = 0; i < passengerCount; i++) if (passengers[i].ticketId == id) return i;
    return -1;
}

int nextBusId() {
    int max = 0;
    for (int i = 0; i < busCount; i++) if (buses[i].busId > max) max = buses[i].busId;
    return max + 1;
}

int nextTicketId() {
    int max = 1000;
    for (int i = 0; i < passengerCount; i++) if (passengers[i].ticketId > max) max = passengers[i].ticketId;
    return max + 1;
}

/* ================= FARE / LOAD HELPERS ================= */
int isPeakHour(int hour) {
    return ((hour >= 7 && hour <= 9) || (hour >= 17 && hour <= 19));
}

int fareMultiplier(int hour, int occupiedPercent) {
    int percent = 100;
    if (isPeakHour(hour)) percent = 125;
    else if (hour >= 11 && hour <= 15) percent = 90;
    if (occupiedPercent >= 75) percent += 10;
    return percent;
}

int occupiedPercentOf(int bi) {
    return (buses[bi].capacity - buses[bi].available) * 100 / buses[bi].capacity;
}

int loadLevel(int percent) {
    return (percent < 40) ? 0 : (percent < 75) ? 1 : 2;
}

int findStopIndex(int ri, char key[]) {
    for (int s = 0; s < routes[ri].stopCount; s++)
        if (strstr(routes[ri].stops[s], key)) return s;
    return -1;
}

/* ================= PRINT HELPERS ================= */
void printBusHeader() {
    printf("\n%-5s %-14s %-6s %-20s %-8s %-9s %-15s\n", "ID", "Reg No", "Route", "Driver", "Capacity", "Available", "Status");
    printf("-----------------------------------------------------------------------------------\n");
}

void printBusRow(int i) {
    printf("%-5d %-14s %-6d %-20s %-8d %-9d %-15s\n", buses[i].busId, buses[i].regNo, buses[i].routeNo, buses[i].driver, buses[i].capacity, buses[i].available, statusNames[buses[i].status]);
}

void printRoute(int i) {
    printf("\nRoute No   : %d\nName       : %s\nBase fare  : Rs.%d\nPer stop   : Rs.%d\nStops (%d) :\n",
           routes[i].routeNo, routes[i].name, routes[i].baseFare, routes[i].ratePerStop, routes[i].stopCount);
    for (int s = 0; s < routes[i].stopCount; s++) printf("   %d. %s\n", s + 1, routes[i].stops[s]);
    if (routes[i].stopCount == 0) printf("   (no stops added yet)\n");
}

void printTicketHeader() {
    printf("\n%-7s %-18s %-4s %-4s %-14s %-14s %-5s %-5s %-6s %-10s\n", "Ticket", "Name", "Age", "Bus", "From", "To", "Hour", "Seats", "Fare", "Status");
    printf("-----------------------------------------------------------------------------------------------\n");
}

void printTicketRow(int i) {
    printf("%-7d %-18s %-4d %-4d %-14s %-14s %-5d %-5d %-6d %-10s\n", passengers[i].ticketId, passengers[i].name, passengers[i].age, passengers[i].busId, passengers[i].fromName, passengers[i].toName, passengers[i].travelHour, passengers[i].seats, passengers[i].fare, ticketStatusNames[passengers[i].status]);
}

/* ================= BUS MANAGEMENT ================= */
void addBus() {
    if (busCount >= MAX_BUSES || routeCount == 0) {
        printf(routeCount == 0 ? "No routes exist yet.\n" : "Bus list is full.\n");
        return;
    }
    struct Bus b;
    b.busId = nextBusId();
    readNonEmpty("Enter registration number: ", b.regNo, 20);
    if (findBusByReg(b.regNo) != -1) { printf("Registration number exists!\n"); return; }
    b.routeNo = readIntInRange("Enter route number: ", 1, 9999);
    if (findRouteIndex(b.routeNo) == -1) { printf("Route does not exist!\n"); return; }
    readNonEmpty("Enter driver name: ", b.driver, 30);
    b.capacity = readIntInRange("Enter capacity (1 to 100): ", 1, 100);
    b.available = b.capacity;
    b.status = 1;
    buses[busCount++] = b;
    saveBuses();
    printf("Bus added! ID = %d\n", b.busId);
}

void viewBuses() {
    if (busCount == 0) { printf("No buses stored yet.\n"); return; }
    printBusHeader();
    for (int i = 0; i < busCount; i++) printBusRow(i);
    printf("Total buses: %d\n", busCount);
}

void searchBus() {
    int found = 0;
    printf("\nSearch bus by:\n 1. Bus ID\n 2. Registration number\n 3. Route number\n");
    int choice = readInt("Enter choice: ");
    if (choice == 1) {
        int idx = findBusIndex(readInt("Enter Bus ID: "));
        if (idx != -1) { printBusHeader(); printBusRow(idx); found = 1; }
    } else if (choice == 2) {
        char reg[20];
        readNonEmpty("Enter registration number: ", reg, 20);
        int idx = findBusByReg(reg);
        if (idx != -1) { printBusHeader(); printBusRow(idx); found = 1; }
    } else if (choice == 3) {
        int r = readInt("Enter route number: ");
        for (int i = 0; i < busCount; i++) {
            if (buses[i].routeNo == r) {
                if (!found) printBusHeader();
                printBusRow(i);
                found++;
            }
        }
    }
    if (!found) printf("No matching bus found.\n");
}

void updateBus() {
    int idx = findBusIndex(readInt("Enter Bus ID to update: "));
    if (idx == -1) { printf("Bus not found.\n"); return; }
    printBusHeader(); printBusRow(idx);
    printf("\nChange:\n 1. Reg No\n 2. Route No\n 3. Driver\n 4. Capacity\n 0. Cancel\n");
    int choice = readInt("Enter choice: "), changed = 0;
    if (choice == 1) {
        char newReg[20];
        readNonEmpty("Enter new registration number: ", newReg, 20);
        if (findBusByReg(newReg) == -1 || findBusByReg(newReg) == idx) {
            strcpy(buses[idx].regNo, newReg); changed = 1;
        } else printf("Reg number already exists!\n");
    } else if (choice == 2) {
        if (buses[idx].capacity != buses[idx].available) printf("Cannot change route: seats booked.\n");
        else {
            int r = readIntInRange("Enter new route: ", 1, 9999);
            if (findRouteIndex(r) != -1) { buses[idx].routeNo = r; changed = 1; }
            else printf("Route does not exist!\n");
        }
    } else if (choice == 3) {
        readNonEmpty("Enter new driver: ", buses[idx].driver, 30); changed = 1;
    } else if (choice == 4) {
        int newCap = readIntInRange("Enter capacity: ", 1, 100);
        int booked = buses[idx].capacity - buses[idx].available;
        if (newCap >= booked) { buses[idx].capacity = newCap; buses[idx].available = newCap - booked; changed = 1; }
        else printf("Cannot reduce below booked seats (%d).\n", booked);
    }
    if (changed) { saveBuses(); printf("Bus updated successfully.\n"); }
}

void deleteBus() {
    int idx = findBusIndex(readInt("Enter Bus ID to delete: "));
    if (idx == -1) { printf("Bus not found.\n"); return; }
    if (buses[idx].capacity != buses[idx].available) { printf("Bus has booked seats.\n"); return; }
    if (!askYesNo("Delete bus? (y/n): ")) return;
    for (int i = idx; i < busCount - 1; i++) buses[i] = buses[i + 1];
    busCount--;
    saveBuses();
    printf("Bus deleted successfully.\n");
}

void busManagementMenu() {
    int choice = -1;
    while (choice != 0) {
        printf("\n===== BUS MANAGEMENT =====\n 1. Add Bus\n 2. View Buses\n 3. Search Bus\n 4. Update Bus\n 5. Delete Bus\n 0. Back\n");
        choice = readInt("Enter choice: ");
        if (choice == 1) addBus();
        else if (choice == 2) viewBuses();
        else if (choice == 3) searchBus();
        else if (choice == 4) updateBus();
        else if (choice == 5) deleteBus();
    }
}

/* ================= BUS DETAILS ================= */
void showBusDetails() {
    int idx = findBusIndex(readInt("Enter Bus ID: "));
    if (idx == -1) { printf("Bus not found.\n"); return; }
    printf("\n----- BUS DETAILS -----\nBus ID: %d\nReg No: %s\nRoute No: %d\n", buses[idx].busId, buses[idx].regNo, buses[idx].routeNo);
    int ri = findRouteIndex(buses[idx].routeNo);
    if (ri != -1) printf("Route name: %s\n", routes[ri].name);
    printf("Driver: %s\nCapacity: %d\nAvailable: %d\nStatus: %s\n", buses[idx].driver, buses[idx].capacity, buses[idx].available, statusNames[buses[idx].status]);
}

void changeBusStatus() {
    int idx = findBusIndex(readInt("Enter Bus ID: "));
    if (idx == -1) { printf("Bus not found.\n"); return; }
    buses[idx].status = readIntInRange("New status (1.Active 2.Maint 3.Off): ", 1, 3);
    saveBuses();
    printf("Status updated.\n");
}

void showSeatAvailability() {
    if (busCount == 0) { printf("No buses stored.\n"); return; }
    printf("\n%-5s %-14s %-8s %-9s %-7s %-15s\n", "ID", "Reg No", "Capacity", "Available", "Booked", "Status");
    for (int i = 0; i < busCount; i++)
        printf("%-5d %-14s %-8d %-9d %-7d %-15s\n", buses[i].busId, buses[i].regNo, buses[i].capacity, buses[i].available, buses[i].capacity - buses[i].available, statusNames[buses[i].status]);
}

void busDetailsMenu() {
    int choice = -1;
    while (choice != 0) {
        printf("\n===== BUS DETAILS =====\n 1. Show Details\n 2. Change Status\n 3. Seat Availability\n 0. Back\n");
        choice = readInt("Enter choice: ");
        if (choice == 1) showBusDetails();
        else if (choice == 2) changeBusStatus();
        else if (choice == 3) showSeatAvailability();
    }
}

/* ================= ROUTE MANAGEMENT ================= */
void addStopsToRoute(int idx) {
    char temp[30];
    while (routes[idx].stopCount < MAX_STOPS) {
        printf("Stop %d name (empty line to finish): ", routes[idx].stopCount + 1);
        readLine(temp, 30);
        if (strlen(temp) == 0) break;
        int dup = 0;
        for (int s = 0; s < routes[idx].stopCount; s++) if (strcmp(routes[idx].stops[s], temp) == 0) dup = 1;
        if (!dup) strcpy(routes[idx].stops[routes[idx].stopCount++], temp);
    }
    saveRoutes();
}

void addRoute() {
    if (routeCount >= MAX_ROUTES) return;
    struct Route r;
    r.routeNo = readIntInRange("Enter route number: ", 1, 9999);
    if (findRouteIndex(r.routeNo) != -1) { printf("Route exists!\n"); return; }
    readNonEmpty("Enter route name: ", r.name, 40);
    r.baseFare = readIntInRange("Base fare: ", 0, 1000);
    r.ratePerStop = readIntInRange("Rate per stop: ", 0, 500);
    r.stopCount = 0;
    routes[routeCount++] = r;
    saveRoutes();
    if (askYesNo("Add stops now? (y/n): ")) addStopsToRoute(routeCount - 1);
}

void addStops() {
    int idx = findRouteIndex(readInt("Enter route number: "));
    if (idx != -1) addStopsToRoute(idx);
    else printf("Route not found.\n");
}

void updateRoute() {
    int idx = findRouteIndex(readInt("Enter route number: "));
    if (idx == -1) { printf("Route not found.\n"); return; }
    printRoute(idx);
    printf("\n1. Name\n2. Base fare\n3. Fare per stop\n4. Rename stop\n");
    int choice = readInt("Enter choice: ");
    if (choice == 1) readNonEmpty("New name: ", routes[idx].name, 40);
    else if (choice == 2) routes[idx].baseFare = readIntInRange("New base fare: ", 0, 1000);
    else if (choice == 3) routes[idx].ratePerStop = readIntInRange("New rate per stop: ", 0, 500);
    else if (choice == 4 && routes[idx].stopCount > 0) {
        int sn = readIntInRange("Stop no: ", 1, routes[idx].stopCount);
        readNonEmpty("New stop name: ", routes[idx].stops[sn - 1], 30);
    }
    saveRoutes();
}

void searchRoute() {
    int choice = readInt("\n1. Number\n2. Name\n3. Stop\nEnter choice: ");
    if (choice == 1) {
        int idx = findRouteIndex(readInt("Enter route number: "));
        if (idx != -1) printRoute(idx);
    } else if (choice == 2 || choice == 3) {
        char key[40];
        readNonEmpty("Enter search term: ", key, 40);
        for (int i = 0; i < routeCount; i++) {
            int match = (choice == 2) ? (strstr(routes[i].name, key) != NULL) : (findStopIndex(i, key) != -1);
            if (match) printRoute(i);
        }
    }
}

void viewRoutes() {
    for (int i = 0; i < routeCount; i++) printRoute(i);
}

void journeyFinder() {
    char fromKey[30], toKey[30];
    readNonEmpty("From stop: ", fromKey, 30);
    readNonEmpty("To stop: ", toKey, 30);
    for (int i = 0; i < routeCount; i++) {
        int f = findStopIndex(i, fromKey), t = findStopIndex(i, toKey);
        if (f != -1 && t != -1 && f < t) {
            int fare = routes[i].baseFare + (t - f) * routes[i].ratePerStop;
            printf("Route %d (%s): %s to %s | Fare Rs.%d\n", routes[i].routeNo, routes[i].name, routes[i].stops[f], routes[i].stops[t], fare);
        }
    }
}

void routeMenu() {
    int choice = -1;
    while (choice != 0) {
        printf("\n===== ROUTE MANAGEMENT =====\n 1. Add Route\n 2. Add Stops\n 3. Update Route\n 4. Search Route\n 5. View Routes\n 6. Find Journey\n 0. Back\n");
        choice = readInt("Enter choice: ");
        if (choice == 1) addRoute();
        else if (choice == 2) addStops();
        else if (choice == 3) updateRoute();
        else if (choice == 4) searchRoute();
        else if (choice == 5) viewRoutes();
        else if (choice == 6) journeyFinder();
    }
}

/* ================= PASSENGER MANAGEMENT ================= */
int calculateFare(int ri, int from, int to, int age, int seats, int hour, int occupiedPercent) {
    int fare = (routes[ri].baseFare + (to - from) * routes[ri].ratePerStop) * fareMultiplier(hour, occupiedPercent) / 100;
    if (age < 12 || age >= 60) fare /= 2;
    return fare * seats;
}

void bookTicket() {
    if (passengerCount >= MAX_TICKETS) return;
    int bi = findBusIndex(readInt("Enter Bus ID: "));
    if (bi == -1 || buses[bi].status != 1 || buses[bi].available == 0) return;
    int ri = findRouteIndex(buses[bi].routeNo);
    if (ri == -1 || routes[ri].stopCount < 2) return;
    printRoute(ri);
    struct Passenger p;
    int from = readIntInRange("Boarding stop no: ", 1, routes[ri].stopCount - 1);
    int to = readIntInRange("Destination stop no: ", from + 1, routes[ri].stopCount);
    readNonEmpty("Passenger name: ", p.name, 30);
    p.age = readIntInRange("Age: ", 1, 120);
    p.seats = readIntInRange("Seats: ", 1, buses[bi].available);
    p.travelHour = readIntInRange("Hour (0-23): ", 0, 23);
    p.fare = calculateFare(ri, from, to, p.age, p.seats, p.travelHour, occupiedPercentOf(bi));
    p.ticketId = nextTicketId();
    p.busId = buses[bi].busId;
    p.routeNo = buses[bi].routeNo;
    strcpy(p.fromName, routes[ri].stops[from - 1]);
    strcpy(p.toName, routes[ri].stops[to - 1]);
    p.status = 1;
    passengers[passengerCount++] = p;
    buses[bi].available -= p.seats;
    savePassengers(); saveBuses();
    printf("Ticket Booked! Ticket ID: %d | Fare: Rs.%d\n", p.ticketId, p.fare);
}

void cancelTicket() {
    int ti = findTicketIndex(readInt("Enter Ticket ID: "));
    if (ti == -1 || passengers[ti].status != 1) return;
    if (!askYesNo("Cancel ticket? (y/n): ")) return;
    passengers[ti].status = 2;
    int bi = findBusIndex(passengers[ti].busId);
    if (bi != -1) buses[bi].available += passengers[ti].seats;
    savePassengers(); saveBuses();
    printf("Ticket cancelled. Refund: Rs.%d\n", passengers[ti].fare * 80 / 100);
}

void fareCalculator() {
    int ri = findRouteIndex(readInt("Enter Route No: "));
    if (ri == -1 || routes[ri].stopCount < 2) return;
    printRoute(ri);
    int from = readIntInRange("From stop no: ", 1, routes[ri].stopCount - 1);
    int to = readIntInRange("To stop no: ", from + 1, routes[ri].stopCount);
    int age = readIntInRange("Age: ", 1, 120);
    int seats = readIntInRange("Seats: ", 1, 50);
    int hour = readIntInRange("Hour: ", 0, 23);
    printf("Total fare: Rs.%d\n", calculateFare(ri, from, to, age, seats, hour, 0));
}

void viewPassengers() {
    int choice = readInt("\n1. Ticket ID\n2. All\n3. By Bus ID\nEnter choice: ");
    if (choice == 1) {
        int ti = findTicketIndex(readInt("Ticket ID: "));
        if (ti != -1) { printTicketHeader(); printTicketRow(ti); }
    } else if (choice == 2 || choice == 3) {
        int bId = (choice == 3) ? readInt("Bus ID: ") : 0, found = 0;
        for (int i = 0; i < passengerCount; i++) {
            if (choice == 2 || passengers[i].busId == bId) {
                if (!found) printTicketHeader();
                printTicketRow(i);
                found++;
            }
        }
    }
}

void passengerMenu() {
    int choice = -1;
    while (choice != 0) {
        printf("\n===== PASSENGER MANAGEMENT =====\n 1. Book Ticket\n 2. Cancel Ticket\n 3. Calculate Fare\n 4. View Details\n 0. Back\n");
        choice = readInt("Enter choice: ");
        if (choice == 1) bookTicket();
        else if (choice == 2) cancelTicket();
        else if (choice == 3) fareCalculator();
        else if (choice == 4) viewPassengers();
    }
}

/* ================= ANALYTICS ================= */
void passengerLoadAnalysis() {
    if (busCount == 0) return;
    printf("\n----- LOAD ANALYSIS PER BUS -----\n");
    for (int i = 0; i < busCount; i++) {
        int pct = occupiedPercentOf(i);
        printf("Bus %d (%s) | Capacity: %d | Booked: %d | Load: %d%% [%s]\n", buses[i].busId, buses[i].regNo, buses[i].capacity, buses[i].capacity - buses[i].available, pct, loadNames[loadLevel(pct)]);
    }
}

void peakHourAnalysis() {
    int seatsAtHour[24] = {0};
    for (int i = 0; i < passengerCount; i++)
        if (passengers[i].status == 1 && passengers[i].travelHour < 24)
            seatsAtHour[passengers[i].travelHour] += passengers[i].seats;

    printf("\n----- PEAK HOUR ANALYSIS -----\n");
    for (int h = 0; h < 24; h++)
        if (seatsAtHour[h] > 0)
            printf("%02d:00 - %02d:59 | Booked Seats: %d\n", h, h, seatsAtHour[h]);
}

void analyticsMenu() {
    int choice = -1;
    while (choice != 0) {
        printf("\n===== ANALYSIS MENU =====\n 1. Load Analysis\n 2. Peak Hour Analysis\n 0. Back\n");
        choice = readInt("Enter choice: ");
        if (choice == 1) passengerLoadAnalysis();
        else if (choice == 2) peakHourAnalysis();
    }
}

/* ================= MAIN ================= */
int main() {
    loadBuses();
    loadRoutes();
    loadPassengers();
    initDefaults();

    printf("\n*****************************************\n*   CITY BUS MANAGEMENT SYSTEM          *\n*****************************************\n");

    int choice = -1;
    while (choice != 0) {
        printf("\n========== MAIN MENU ==========\n 1. Bus Management\n 2. Bus Details\n 3. Route Management\n 4. Passenger Management\n 5. Analysis and Reports\n 0. Exit\n");
        choice = readInt("Enter choice: ");
        if (choice == 1) busManagementMenu();
        else if (choice == 2) busDetailsMenu();
        else if (choice == 3) routeMenu();
        else if (choice == 4) passengerMenu();
        else if (choice == 5) analyticsMenu();
    }
    return 0;
}
