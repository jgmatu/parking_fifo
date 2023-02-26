#include <parking.h>
#include <stdio.h>
#include <stdlib.h>

static parking_t g_parking;

void print_parking()
{
    char buffer[1 * 1024] = { 0 };
    int16_t nwrite = 0;

    for (uint16_t i = 0; i < MAX_SLOTS; ++i) {
        nwrite += snprintf(&buffer[nwrite], 1 * 1024 - nwrite, "[%d]", g_parking.slots[i]);
    }
    nwrite += snprintf(&buffer[nwrite], 1 * 1024 - nwrite, " - > free slots: %d", g_parking.nslots);
    fprintf(stdout, "%s\n", buffer);
}

void init_parking()
{
    for (uint8_t i = 0; i < MAX_SLOTS; ++i) {
        g_parking.slots[i] = -1;
    }
    g_parking.nslots = MAX_SLOTS;
}

static int16_t isEmptyTruckSlot(uint16_t slot)
{
    uint16_t total = 0;

    for (uint16_t i = slot; i < MAX_SLOTS && total != TRUCK_SIZE; ++i) {
        if (g_parking.slots[i] >= 0) {
            break;
        }
        ++total;
    }
    return total == TRUCK_SIZE ? slot : -1;
}

int16_t entry_parking(vehicle_t *vehicle)
{
    int16_t idx = -1;

    switch (vehicle->type) {
        case CAR:
            for (uint16_t i = 0; i < MAX_SLOTS && idx < 0; ++i) {
                if (g_parking.slots[i] >= 0) {
                    continue;
                }
                g_parking.slots[i] = vehicle->id;
                --g_parking.nslots;
                idx = i;
            }
            break;
        case TRUCK:
            for (uint16_t i = 0; i < MAX_SLOTS && idx < 0; ++i) {
                if ((idx = isEmptyTruckSlot(i)) < 0) continue;

                fprintf(stdout, "Parking slot %d\n", idx);
                for (uint16_t j = 0; j < TRUCK_SIZE; ++j) {
                    g_parking.slots[idx + j] = vehicle->id;
                }
                g_parking.nslots -= TRUCK_SIZE;
            }
            break;
        default:
            break;
    }
    vehicle->slot = idx;
    return idx;
}

void exit_parking(vehicle_t *vehicle)
{
    switch (vehicle->type) {
        case CAR:
            g_parking.slots[vehicle->slot] = -1;
            ++g_parking.nslots;
            break;
        case TRUCK:
            for (uint16_t i = vehicle->slot; i < vehicle->slot + TRUCK_SIZE; ++i) {
                g_parking.slots[i] = -1;
            }
            g_parking.nslots += TRUCK_SIZE;
            break;
        default:
            break;
    }
}
