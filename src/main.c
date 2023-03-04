#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <stdint.h>

#include <queue.h>
#include <parking.h>

pthread_barrier_t barrier;

static queue_control_t g_queue_cars;
static queue_control_t g_queue_trucks;
static parking_control_t g_parking;

// TODO: FIFO Queue control threads
void * control_task(void * arg)
{
    pthread_mutex_lock(&g_parking.mtx);

    for (;;) {
        pthread_cond_wait(&g_parking.cond, &g_parking.mtx);

        if (g_parking.nslots >= TRUCK_SIZE) {
            pthread_mutex_lock(&g_queue_trucks.mtx);
            if (g_queue_trucks.queue.size == 0) {
                notify_queue(&g_queue_cars);
            }
            pthread_mutex_unlock(&g_queue_trucks.mtx);
            notify_queue(&g_queue_trucks);
        } else {
            notify_queue(&g_queue_cars);
        }

    }
    return NULL;
}

typedef struct parking_args_t {
    uint16_t id;
    type_t type;
} parking_args_t;

void * task(void *arg)
{
    parking_args_t *parking_args = (parking_args_t *) arg;
    vehicle_t vehicle;

    vehicle.id = parking_args->id;
    vehicle.type = parking_args->type;
    vehicle.slot = -1;
#if 0
    pthread_mutex_init(&vehicle.mtx, NULL);
    pthread_cond_init(&vehicle.cond, NULL);
    vehicle.wake_up = 0;
#endif
    free(parking_args);

    for (;;) {

        while ((vehicle.slot = entry_parking(&g_parking, &vehicle)) < 0) {
            switch (vehicle.type)
            {
            case TRUCK:
                entry_queue(&g_queue_trucks, &vehicle);
                break;
            case CAR:
                entry_queue(&g_queue_cars, &vehicle);
                break;
            }
        }

        switch (vehicle.type)
        {
        case TRUCK:
            exit_queue(&g_queue_trucks, &vehicle);
            break;
        case CAR:
            exit_queue(&g_queue_cars, &vehicle);
           break;
        }
 
        fprintf(stdout,"ENTRADA: %s: %d plaza : %d\n",
            (vehicle.type == TRUCK) ? "camion" : "coche", vehicle.id, vehicle.slot);
        print_parking(&g_parking);

        sleep(rand() % 10 + 10);

        exit_parking(&g_parking, &vehicle);

        fprintf(stdout,"SALIDA: %s: %d plaza : %d\n",
            (vehicle.type == TRUCK) ? "camion" : "coche", vehicle.id, vehicle.slot);
        print_parking(&g_parking);

        pthread_cond_signal(&g_parking.cond);
        pthread_barrier_wait(&barrier);

        vehicle.slot = -1;
    }
    return NULL;
}

int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    pthread_t thread[NUM_VEHICLES];
    pthread_t control_th;

    pthread_barrier_init(&barrier, NULL, NUM_VEHICLES);

    init_parking(&g_parking);
    init_queue(&g_queue_cars, CAR);
    init_queue(&g_queue_trucks, TRUCK);

    pthread_create(&control_th, NULL, control_task, NULL);

    for (uint8_t i = 0; i < NUM_VEHICLES; ++i) {
        parking_args_t *parking_arg = NULL;

        if ((parking_arg = calloc(1, sizeof(parking_args_t))) == NULL) {
            err(1, "Error allocating argument memory! %s", strerror(errno));
        }

        parking_arg->id = i;
        parking_arg->type = (i % 2 == 0) ? TRUCK : CAR;
        pthread_create(&thread[i], NULL, task, parking_arg);
    }

    for (uint8_t i = 0; i < NUM_VEHICLES; ++i) {
        pthread_join(thread[i], NULL);
    }
    pthread_join(control_th, NULL);

    print_parking(&g_parking);
    return 0;
}
