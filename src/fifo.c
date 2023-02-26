#include <types.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <err.h>
#include <string.h>

static queue_control_t g_queue;

void print_queue()
{
    int16_t nwrite = 0;
    char buffer[1 * 1024] = { 0 };
    node_t *node = g_queue.queue.first;

    nwrite += snprintf(&buffer[nwrite], 1 * 1024 - nwrite, "%s: (%ld): ", "Queue", g_queue.queue.size);
    while (node) {
        nwrite += snprintf(&buffer[nwrite], 1 * 1024 - nwrite, "%d", node->vehicle->id);
        if (node->next)
            nwrite += snprintf(&buffer[nwrite], 1 * 1024 - nwrite, "%s", ",");
        node = node->next;
    }
    if (g_queue.queue.first)
        fprintf(stdout, "%s\n", buffer);
}

void push_queue(vehicle_t *vehicle)
{
    node_t **ptr_node = &g_queue.queue.first;

    while (*ptr_node) ptr_node = &(*ptr_node)->next;

    if ((*ptr_node = (node_t * ) calloc(1, sizeof(node_t))) == NULL) {
        err(1, "Error allocating node memory %s", strerror(errno));
    }
    (*ptr_node)->vehicle = vehicle;
    g_queue.queue.size++;
}

void del_queue(const vehicle_t *vehicle)
{
    node_t *prev = NULL;
    node_t *del = g_queue.queue.first;
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
        if (del == g_queue.queue.first) {
            g_queue.queue.first = g_queue.queue.first->next;
        }
        g_queue.queue.size--;
        free(del);
    }
}

vehicle_t * pop_queue()
{
    node_t *del = g_queue.queue.first;
    vehicle_t *vehicle_pop = NULL;

    if (g_queue.queue.first) {
        vehicle_pop = g_queue.queue.first->vehicle;
        g_queue.queue.first = g_queue.queue.first->next;
        g_queue.queue.size--;
    }
    free(del);
    return vehicle_pop;
}

int16_t is_first_queue(vehicle_t *vehicle)
{
    return g_queue.queue.first && g_queue.queue.first->vehicle->id == vehicle->id;
}