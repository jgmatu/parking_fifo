#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <stdint.h>

#include <fifo.h>

static parking_t g_parking;

pthread_barrier_t barrier;

// Problema: Me voy del parking, aviso de sitio libre, no soy el primero me duermo.
// El primero no encontro sitio se espera.
// Me quedo sin signals, por que no despierto al primero de la lista o no encontro sitio.

// Solucion despertar siempre al primero de la lista...
void * control(void * arg)
{
    return NULL;
}

static void exit_fifo(vehicle_t *vehicle)
{
    del_fifo(vehicle);
    print_fifo();
}

static void entry_fifo(vehicle_t *vehicle)
{
    push_fifo(vehicle);
    print_fifo();
}

static void print_parking()
{
    char buffer[1 * 1024] = { 0 };
    int16_t nwrite = 0;

    for (uint16_t i = 0; i < MAX_SLOTS; ++i) {
        nwrite += snprintf(&buffer[nwrite], 1 * 1024 - nwrite, "[%d]", g_parking.slots[i]);
    }
    nwrite += snprintf(&buffer[nwrite], 1 * 1024 - nwrite, " - > free slots: %d", g_parking.nslots);
    fprintf(stdout, "%s\n", buffer);
}

static int16_t entry_parking(vehicle_t *vehicle)
{
    int16_t idx = -1;

    switch (vehicle->type) {
        case CAR:
            for (uint16_t i = 0; i < MAX_SLOTS && idx < 0; ++i) {
                if (g_parking.slots[i] >= 0) {
                    continue;
                }
                g_parking.slots[i] = vehicle->id;
                --g_parking.nslots;
                idx = i;
            }
            break;
        case TRUCK:
            for (uint16_t i = 0; i < MAX_SLOTS - 1 && idx < 0; ++i) {
                if (g_parking.slots[i] >= 0 || g_parking.slots[i + 1] >= 0) {
                    continue;
                }
                g_parking.slots[i] = vehicle->id;
                g_parking.slots[i + 1] = vehicle->id;
                --g_parking.nslots;
                --g_parking.nslots;
                idx = i;
            }
            break;
        default:
            break;
    }
    return idx;
}

static void exit_parking(vehicle_t *vehicle)
{
    switch (vehicle->type) {
        case CAR:
            g_parking.slots[vehicle->slot] = -1;
            ++g_parking.nslots;
            break;
        case TRUCK:
            g_parking.slots[vehicle->slot] = -1;
            g_parking.slots[vehicle->slot + 1] = -1;
            ++g_parking.nslots;
            ++g_parking.nslots;
            break;
        default:
            break;
    }
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
    pthread_mutex_init(&vehicle.mtx, NULL);
    pthread_cond_init(&vehicle.cond, NULL);
    vehicle.wake_up = 0;

    free(parking_args);
    for (;;) {

        pthread_mutex_lock(&g_parking.mtx);
        while ((vehicle.slot = entry_parking(&vehicle)) < 0) {
            entry_fifo(&vehicle);
            if (vehicle.type == TRUCK)
                pthread_cond_signal(&g_parking.cond);
            pthread_cond_wait(&g_parking.cond, &g_parking.mtx);
            exit_fifo(&vehicle);
        }

        fprintf(stdout,"ENTRADA: %s: %d plaza : %d\n",
            (vehicle.type == TRUCK) ? "camion" : "coche", vehicle.id, vehicle.slot);

        print_parking();
        pthread_mutex_unlock(&g_parking.mtx);

        sleep(rand() % 30 + 60);

        pthread_mutex_lock(&g_parking.mtx);
        exit_parking(&vehicle);

        fprintf(stdout,"SALIDA: %s: %d plaza : %d\n",
            (vehicle.type == TRUCK) ? "camion" : "coche", vehicle.id, vehicle.slot);

        print_parking();
        pthread_mutex_unlock(&g_parking.mtx);
        pthread_cond_signal(&g_parking.cond);

        vehicle.slot = -1;

        pthread_barrier_wait(&barrier);
    }
    return NULL;
}

int main(int argc, char **argv)
{
    pthread_t thread[MAX_TRUCKS + MAX_CARS];
    pthread_t control_th;

    pthread_mutex_init(&g_parking.mtx, NULL);
    pthread_cond_init(&g_parking.cond, NULL);
    pthread_barrier_init(&barrier, NULL, MAX_TRUCKS + MAX_CARS);

    for (uint8_t i = 0; i < MAX_SLOTS; ++i) {
        g_parking.slots[i] = -1;
    }
    g_parking.nslots = MAX_SLOTS;
    print_parking();

    pthread_create(&control_th, NULL, control, NULL);

    for (uint8_t i = 0; i < MAX_TRUCKS + MAX_CARS; ++i) {
        parking_args_t *parking_arg = NULL;

        if ((parking_arg = calloc(1, sizeof(parking_args_t))) == NULL) {
            err(1, "Error allocating argument memory! %s", strerror(errno));
        }

        parking_arg->id = i;
        parking_arg->type = (i % 2 == 0) ? TRUCK : CAR;
        pthread_create(&thread[i], NULL, task, parking_arg);
    }

    for (uint8_t i = 0; i < MAX_CARS + MAX_TRUCKS; ++i) {
        pthread_join(thread[i], NULL);
    }
    pthread_join(control_th, NULL);
    print_parking();
}
