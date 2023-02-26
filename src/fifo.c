#include <types.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <err.h>
#include <string.h>

static fifo_control_t g_fifo;

void print_fifo()
{
    int16_t nwrite = 0;
    char buffer[1 * 1024] = { 0 };
    node_t *node = g_fifo.fifo.first;

    nwrite += snprintf(&buffer[nwrite], 1 * 1024 - nwrite, "%s: (%ld): ", "Queue", g_fifo.fifo.size);
    while (node) {
        nwrite += snprintf(&buffer[nwrite], 1 * 1024 - nwrite, "%d,", node->vehicle->id);
        node = node->next;
    }
    if (g_fifo.fifo.first)
        fprintf(stdout, "%s\n", buffer);
}

void push_fifo(vehicle_t *vehicle)
{
    node_t **ptr_node = &g_fifo.fifo.first;

    while (*ptr_node) ptr_node = &(*ptr_node)->next;

    if ((*ptr_node = (node_t * ) calloc(1, sizeof(node_t))) == NULL) {
        err(1, "Error allocating node memory %s", strerror(errno));
    }
    (*ptr_node)->vehicle = vehicle;
    g_fifo.fifo.size++;
}

void del_fifo(const vehicle_t *vehicle)
{
    node_t *prev = NULL;
    node_t *del = g_fifo.fifo.first;
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
        if (del == g_fifo.fifo.first) {
            g_fifo.fifo.first = g_fifo.fifo.first->next;
        }
        g_fifo.fifo.size--;
        free(del);
    }
}

vehicle_t * pop_fifo()
{
    node_t *del = g_fifo.fifo.first;
    vehicle_t *vehicle_pop = NULL;

    if (g_fifo.fifo.first) {
        vehicle_pop = g_fifo.fifo.first->vehicle;
        g_fifo.fifo.first = g_fifo.fifo.first->next;
        g_fifo.fifo.size--;
    }
    free(del);
    return vehicle_pop;
}

int16_t is_first_fifo(vehicle_t *vehicle)
{
    return g_fifo.fifo.first && g_fifo.fifo.first->vehicle->id == vehicle->id;
}