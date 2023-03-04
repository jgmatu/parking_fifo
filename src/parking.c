#include <parking.h>
#include <stdio.h>
#include <stdlib.h>

void init_parking(parking_control_t *parking)
{
    for (uint8_t i = 0; i < MAX_SLOTS; ++i) {
        parking->slots[i] = -1;
    }
    parking->nslots = MAX_SLOTS;
    pthread_mutex_init(&parking->mtx, NULL);
    pthread_cond_init(&parking->cond, NULL);
}

static inline int16_t isEmptyTruckSlot(const parking_control_t *parking, uint16_t slot)
{
    uint16_t total = 0;

    for (uint16_t i = slot; i < MAX_SLOTS && total != TRUCK_SIZE; ++i) {
        if (parking->slots[i] >= 0) {
            break;
        }
        ++total;
    }
    return total == TRUCK_SIZE ? slot : -1;
}

int16_t entry_parking(parking_control_t *parking, vehicle_t *vehicle)
{
    int16_t idx = -1;

    pthread_mutex_lock(&parking->mtx);
    switch (vehicle->type) {
        case CAR:
            for (uint16_t i = 0; i < MAX_SLOTS && idx < 0; ++i) {
                if (parking->slots[i] >= 0) {
                    continue;
                }
                parking->slots[i] = vehicle->id;
                --parking->nslots;
                idx = i;
            }
            break;
        case TRUCK:
            for (uint16_t i = 0; i < MAX_SLOTS && idx < 0; ++i) {
                if ((idx = isEmptyTruckSlot(parking, i)) < 0) continue;

                fprintf(stdout, "Parking slot %d\n", idx);
                for (uint16_t j = 0; j < TRUCK_SIZE; ++j) {
                    parking->slots[idx + j] = vehicle->id;
                }
                parking->nslots -= TRUCK_SIZE;
            }
            break;
        default:
            break;
    }
    pthread_mutex_unlock(&parking->mtx);

    vehicle->slot = idx;
    return idx;
}

void exit_parking(parking_control_t *parking, vehicle_t *vehicle)
{
    pthread_mutex_lock(&parking->mtx);

    switch (vehicle->type) {
        case CAR:
            parking->slots[vehicle->slot] = -1;
            ++parking->nslots;
            break;
        case TRUCK:
            for (uint16_t i = vehicle->slot; i < vehicle->slot + TRUCK_SIZE; ++i) {
                parking->slots[i] = -1;
            }
            parking->nslots += TRUCK_SIZE;
            break;
        default:
            break;
    }
    pthread_mutex_unlock(&parking->mtx);
}

void print_parking(parking_control_t *parking)
{
    char buffer[1 * 1024] = { 0 };
    int16_t nwrite = 0;

    pthread_mutex_lock(&parking->mtx);
    for (uint16_t i = 0; i < MAX_SLOTS; ++i) {
        nwrite += snprintf(&buffer[nwrite], 1 * 1024 - nwrite, "[%d]", parking->slots[i]);
    }
    nwrite += snprintf(&buffer[nwrite], 1 * 1024 - nwrite, " - > free slots: %d", parking->nslots);
    pthread_mutex_unlock(&parking->mtx);

    fprintf(stdout, "%s\n", buffer);
}


