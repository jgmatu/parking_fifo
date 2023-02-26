#include <pthread.h>
#include <stdint.h>

#ifndef TYPES_H
#define TYPES_H

#define MAX_SLOTS 10
#define NUM_VEHICLES 10
#define TRUCK_SIZE 10

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

typedef struct node_t {
    vehicle_t *vehicle;
    struct node_t *next;
} node_t;

typedef struct list_t {
    struct node_t *first;
    size_t size;
} list_t;

typedef struct queue_control_t {
    list_t queue;
    pthread_mutex_t mtx;
    pthread_cond_t cond;
} queue_control_t;

typedef struct parking_t {
    int16_t slots[MAX_SLOTS];
    uint16_t nslots;
    pthread_mutex_t mtx;
    pthread_cond_t cond;
} parking_t;
#endif