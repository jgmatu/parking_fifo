#include <pthread.h>
#include <stdint.h>

#ifndef TYPES_H
#define TYPES_H

#define MAX_SLOTS 29
#define NUM_VEHICLES 20
#define TRUCK_SIZE 2

typedef enum  {
    CAR = 0,
    TRUCK
} type_t;

typedef struct vehicle_t {
    uint16_t id;
    type_t type;
    int16_t slot;
#if 0
    pthread_mutex_t mtx;
    pthread_cond_t cond;
    uint8_t wake_up;
#endif
} vehicle_t;

#endif