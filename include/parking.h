#include <types.h>

typedef struct parking_control_t {
    int16_t slots[MAX_SLOTS];
    uint16_t nslots;
    pthread_mutex_t mtx;
    pthread_cond_t cond;
    uint16_t truck_fail;
} parking_control_t;

void print_parking(parking_control_t *parking);

void init_parking(parking_control_t *parking);

int16_t entry_parking(parking_control_t *parking, vehicle_t *vehicle);

void exit_parking(parking_control_t *parking, vehicle_t *vehicle);