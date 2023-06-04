#include <types.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <err.h>
#include <string.h>

#include <queue.h>

static int search_queue(queue_control_t *queue, vehicle_control_t *vehicle)
{
    node_t *node = queue->queue.first;
    int idx = -1;
    uint16_t pos = 0;

    while (node) {
        if (node->vehicle->id == vehicle->id) {
            idx = pos;
            break;
        }
        pos = pos + 1;
        node = node->next;
    }
    return idx;
}

void init_queue(queue_control_t *queue, type_t type)
{
    queue->queue.first = NULL;
    queue->queue.size = 0;
    queue->queue.type = type;
    pthread_mutex_init(&queue->mtx, NULL);
    pthread_cond_init(&queue->cond, NULL);
}

void exit_wait_queue(queue_control_t *queue, vehicle_control_t *vehicle)
{
    pthread_mutex_lock(&queue->mtx);

    del_queue(queue, vehicle);
    print_queue(queue);

    pthread_mutex_unlock(&queue->mtx);
    pthread_cond_signal(&queue->cond);
}

void entry_wait_queue(queue_control_t *queue, vehicle_control_t *vehicle)
{
    pthread_mutex_lock(&queue->mtx);
    if (search_queue(queue, vehicle) < 0) {
        push_queue(queue, vehicle);
    }
    print_queue(queue);
#if 0
    pthread_mutex_lock(&vehicle->mtx);
    vehicle->wake_up = 0;
    while(vehicle->wake_up == 0) {
        pthread_cond_wait(&vehicle->cond, &vehicle->mtx);
    }
    pthread_mutex_unlock(&vehicle->mtx);
#endif

    pthread_cond_wait(&queue->cond, &queue->mtx);
    pthread_mutex_unlock(&queue->mtx);
}

void push_queue(queue_control_t *queue, vehicle_control_t *vehicle)
{
    node_t **ptr_node = &queue->queue.first;

    while (*ptr_node) ptr_node = &(*ptr_node)->next;

    if ((*ptr_node = (node_t *) calloc(1, sizeof(node_t))) == NULL) {
        err(1, "Error allocating node memory %s", strerror(errno));
    }
    (*ptr_node)->vehicle = vehicle;
    queue->queue.size++;
}

void del_queue(queue_control_t *queue, const vehicle_control_t *vehicle)
{
    node_t *prev = NULL;
    node_t *del = queue->queue.first;
    int delete = 0;

    while (del && !delete) {
        if (del->vehicle->id == vehicle->id) {
            delete = 1;
        } else {
            prev = del;
            del = del->next;
        }
    }

    if (delete) {
        if (prev) {
            prev->next = del->next;
        }
        if (del == queue->queue.first) {
            queue->queue.first = queue->queue.first->next;
        }
        queue->queue.size--;
        free(del);
    }
}

vehicle_control_t * pop_queue(queue_control_t *queue)
{
    node_t *del = queue->queue.first;
    vehicle_control_t *vehicle_pop = NULL;

    if (queue->queue.first) {
        vehicle_pop = queue->queue.first->vehicle;
        queue->queue.first = queue->queue.first->next;
        queue->queue.size--;
    }
    free(del);
    return vehicle_pop;
}

void notify_queue(queue_control_t *queue)
{
#if 0
    vehicle_control_t *vehicle = NULL;

    pthread_mutex_lock(&queue->mtx);
    if ((vehicle = pop_queue(queue)) != NULL) {
        pthread_mutex_lock(&vehicle->mtx);
        vehicle->wake_up = 1;
        pthread_mutex_unlock(&vehicle->mtx);
        pthread_cond_signal(&vehicle->cond);
    } else {
        pthread_mutex_unlock(&queue->mtx);
    }
#else
    pthread_mutex_lock(&queue->mtx);
    if (queue->queue.size > 0) {
        pthread_mutex_unlock(&queue->mtx);
        pthread_cond_signal(&queue->cond);
    } else {
        pthread_mutex_unlock(&queue->mtx);
    }
#endif
}

void print_queue(const queue_control_t *queue)
{
    int16_t nwrite = 0;
    char buffer[1 * 1024] = { 0 };
    node_t *node = queue->queue.first;

    nwrite += snprintf(&buffer[nwrite], 1 * 1024 - nwrite, "Queue %s: (%ld): ",
        (queue->queue.type == TRUCK) ? "Trucks" : "Cars" , queue->queue.size);
    while (node) {
        nwrite += snprintf(&buffer[nwrite], 1 * 1024 - nwrite, "%d", node->vehicle->id);
        if (node->next)
            nwrite += snprintf(&buffer[nwrite], 1 * 1024 - nwrite, "%s", ",");
        node = node->next;
    }
    if (!queue->queue.first)
        snprintf(buffer, 1 * 1024, "Queue %s: (Empty)", (queue->queue.type == TRUCK) ? "Trucks" : "Cars");
    fprintf(stdout, "%s\n", buffer);
}
