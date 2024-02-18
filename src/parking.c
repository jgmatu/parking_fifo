#include <parking.h>
#include <stdio.h>
#include <stdlib.h>

static inline void print_parking(parking_control_t *parking)
{
    char buffer[1 * 1024] = { [ 0 ... sizeof(buffer) - 1 ] = 0 };
    int16_t nwrite = 0;

    for (uint16_t i = 0; i < MAX_SLOTS; ++i)
    {
        for (uint16_t j = 0; j < MAX_SLOTS; ++j)
        {
            nwrite += snprintf(&buffer[nwrite], 1 * 1024 - nwrite, "[%d]", parking->row[i].slot[j]);
        }
        nwrite += snprintf(&buffer[nwrite], 1 * 1024 - nwrite, " - > free slot: %d", parking->row[i].nslots);
        nwrite += snprintf(&buffer[nwrite], 1 * 1024 - nwrite, "%s\n", "");
    }

    fprintf(stdout, "%s\n", buffer);
}

static inline int16_t isEmptyTruckSlot(const parking_control_t *parking, uint16_t nrow, uint16_t slot)
{
    uint16_t total = 0;

    for (uint16_t i = slot; i < MAX_SLOTS && total != TRUCK_SIZE; ++i)
    {
        if (parking->row[nrow].slot[i] >= 0)
        {
            break;
        }
        ++total;
    }
    return total == TRUCK_SIZE ? slot : -1;
}

void init_parking(parking_control_t *parking)
{
    pthread_mutex_init(&parking->mtx, NULL);
    pthread_cond_init(&parking->cond, NULL);
    for (uint32_t i = 0; i < MAX_SLOTS; ++i)
    {
        for (uint32_t j = 0; j < MAX_SLOTS; ++j)
        {
            parking->row[i].slot[j] = -1;
        }
        parking->row[i].nslots = MAX_SLOTS;
    }
}

int16_t entry_parking(parking_control_t *parking, vehicle_control_t *vehicle)
{
    int16_t idx = -1;
    int16_t nrow = -1;

    pthread_mutex_lock(&parking->mtx);
    switch (vehicle->type) {
        case CAR:
            for (uint16_t i = 0; i < MAX_SLOTS && idx < 0; ++i)
            {
                for (uint16_t j = 0; j < MAX_SLOTS && idx < 0; ++j)
                {
                    if (parking->row[i].slot[j] >= 0) {
                        continue;
                    }
                    parking->row[i].slot[j] = vehicle->id;
                    --parking->row[i].nslots;
                    idx = j;
                    nrow = i;
                }
            }
            break;
        case TRUCK:
            for (uint16_t i = 0; i < MAX_SLOTS && idx < 0; ++i)
            {
                for (uint16_t j = 0; j < MAX_SLOTS && idx < 0; ++j)
                {
                    if ((idx = isEmptyTruckSlot(parking, i, j)) < 0) {
                        continue;
                    }
                    for (uint16_t k = 0; k < TRUCK_SIZE; ++k) {
                        parking->row[i].slot[idx + k] = vehicle->id;
                    }
                    parking->row[i].nslots -= TRUCK_SIZE;
                    nrow = i;
                }
            }
            break;
        default:
            break;
    }
    vehicle->slot = idx;
    vehicle->nrow = nrow;

    if (idx >= 0) {
        fprintf(stdout,"ENTRADA: %s: %d plaza : %d\n",
            (vehicle->type == TRUCK) ? "camion" : "coche", vehicle->id, vehicle->slot);
        print_parking(parking);
    }
    pthread_mutex_unlock(&parking->mtx);
    return idx;
}

void exit_parking(parking_control_t *parking, vehicle_control_t *vehicle)
{
    pthread_mutex_lock(&parking->mtx);
    switch (vehicle->type) {
        case CAR:
            parking->row[vehicle->nrow].slot[vehicle->slot] = -1;
            ++parking->row[vehicle->nrow].nslots;
            break;
        case TRUCK:
            for (uint16_t i = vehicle->slot; i < vehicle->slot + TRUCK_SIZE; ++i) {
                parking->row[vehicle->nrow].slot[i] = -1;
            }
            parking->row[vehicle->nrow].nslots += TRUCK_SIZE;
            break;
        default:
            break;
    }
    fprintf(stdout,"SALIDA: %s: %d plaza : %d\n",
        (vehicle->type == TRUCK) ? "camion" : "coche", vehicle->id, vehicle->slot);
    print_parking(parking);
    pthread_mutex_unlock(&parking->mtx);
}