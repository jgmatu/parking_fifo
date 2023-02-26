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

pthread_mutex_t mtx;
pthread_cond_t cond;

// TODO: FIFO Queue control threads
#if 0
void * control(void * arg)
{
    return NULL;
}
#endif

static void exit_queue(vehicle_t *vehicle)
{
    del_queue(vehicle);
    print_queue();
}

static void entry_queue(vehicle_t *vehicle)
{
    push_queue(vehicle);
    print_queue();
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

        pthread_mutex_lock(&mtx);
        while ((vehicle.slot = entry_parking(&vehicle)) < 0) {
            entry_queue(&vehicle);
            pthread_cond_wait(&cond, &mtx);
            exit_queue(&vehicle);
        }

        fprintf(stdout,"ENTRADA: %s: %d plaza : %d\n",
            (vehicle.type == TRUCK) ? "camion" : "coche", vehicle.id, vehicle.slot);

        print_parking();
        pthread_mutex_unlock(&mtx);

        sleep(rand() % 10 + 10);

        pthread_mutex_lock(&mtx);
        exit_parking(&vehicle);

        fprintf(stdout,"SALIDA: %s: %d plaza : %d\n",
            (vehicle.type == TRUCK) ? "camion" : "coche", vehicle.id, vehicle.slot);

        print_parking();
        pthread_mutex_unlock(&mtx);
        pthread_cond_signal(&cond);

        vehicle.slot = -1;

        pthread_barrier_wait(&barrier);
    }
    return NULL;
}

int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    pthread_t thread[NUM_VEHICLES];
#if 0
    pthread_t control_th;
#endif
    pthread_mutex_init(&mtx, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_barrier_init(&barrier, NULL, NUM_VEHICLES);

    init_parking();

#if 0
    pthread_create(&control_th, NULL, control, NULL);
#endif

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
#if 0
    pthread_join(control_th, NULL);
#endif
    print_parking();
}
