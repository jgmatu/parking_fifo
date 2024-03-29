#include <pthread.h>
#include <stdint.h>

#ifndef TYPES_H
#define TYPES_H

#define MAX_SLOTS 10
#define NUM_VEHICLES 30
#define TRUCK_SIZE 9

typedef enum  {
    CAR = 0,
    TRUCK
} type_t;

typedef struct vehicle_control_t {
    uint16_t id;
    type_t type;
    int16_t slot;
    int16_t nrow;
#if 1
    pthread_mutex_t mtx;
    pthread_cond_t cond;
    uint8_t wake_up;
#endif
} vehicle_control_t;

#endif