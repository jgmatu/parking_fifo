#include <types.h>

typedef struct parking_control_t {
    struct parking_data
    {
        int16_t slot[MAX_SLOTS];
        int16_t nslots;
    };
    struct parking_data row[MAX_SLOTS];
    pthread_mutex_t mtx;
    pthread_cond_t cond;
    int16_t truck_fail;
} parking_control_t;

void init_parking(parking_control_t *parking);

int16_t entry_parking(parking_control_t *parking, vehicle_control_t *vehicle);

void exit_parking(parking_control_t *parking, vehicle_control_t *vehicle);