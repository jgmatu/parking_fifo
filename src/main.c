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

    for (;;)
    {
        pthread_cond_wait(&g_parking.cond, &g_parking.mtx);

        if (g_parking.row[0].nslots >= TRUCK_SIZE && !g_parking.truck_fail)
        {
            pthread_mutex_lock(&g_queue_trucks.mtx);
            if (g_queue_trucks.queue.size == 0)
            {
                notify_queue(&g_queue_cars);
            }
            pthread_mutex_unlock(&g_queue_trucks.mtx);
            notify_queue(&g_queue_trucks);
        }
        else
        {
            notify_queue(&g_queue_cars);
        }
        g_parking.truck_fail = 0;
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
    vehicle_control_t vehicle;

    vehicle.id = parking_args->id;
    vehicle.type = parking_args->type;
    vehicle.slot = -1;
#if 0
    pthread_mutex_init(&vehicle.mtx, NULL);
    pthread_cond_init(&vehicle.cond, NULL);
    vehicle.wake_up = 0;
#endif
    free(parking_args);

    for (;;)
    {
        while ((vehicle.slot = entry_parking(&g_parking, &vehicle)) < 0)
        {
            switch (vehicle.type)
            {
            case TRUCK:
                pthread_mutex_lock(&g_parking.mtx);
                if (g_parking.row[0].nslots >= TRUCK_SIZE)
                {
                    g_parking.truck_fail = 1;
                }
                pthread_mutex_unlock(&g_parking.mtx);
                pthread_cond_signal(&g_parking.cond);
                entry_wait_queue(&g_queue_trucks, &vehicle);
                break;
            case CAR:
                entry_wait_queue(&g_queue_cars, &vehicle);
                break;
            }
        }

        switch (vehicle.type)
        {
        case TRUCK:
            exit_wait_queue(&g_queue_trucks, &vehicle);
            break;
        case CAR:
            exit_wait_queue(&g_queue_cars, &vehicle);
           break;
        }

        sleep(rand() % 1 + 5);
        exit_parking(&g_parking, &vehicle);

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

    pthread_t threads[NUM_VEHICLES];
    pthread_t control;

    pthread_barrier_init(&barrier, NULL, NUM_VEHICLES);

    init_parking(&g_parking);
    init_queue(&g_queue_cars, CAR);
    init_queue(&g_queue_trucks, TRUCK);

    pthread_create(&control, NULL, control_task, NULL);

    for (uint8_t i = 0; i < NUM_VEHICLES; ++i)
    {
        parking_args_t *parking_arg = NULL;

        if ((parking_arg = calloc(1, sizeof(parking_args_t))) == NULL)
        {
            err(1, "Error allocating argument memory! %s", strerror(errno));
        }
        parking_arg->id = i;
        parking_arg->type = (i % 2 == 0) ? TRUCK : CAR;
        pthread_create(&threads[i], NULL, task, parking_arg);
    }

    for (uint8_t i = 0; i < NUM_VEHICLES; ++i)
    {
        pthread_join(threads[i], NULL);
    }
    pthread_join(control, NULL);
    return 0;
}
