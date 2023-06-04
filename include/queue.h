#include <types.h>

typedef struct node_t {
    vehicle_control_t *vehicle;
    struct node_t *next;
} node_t;

typedef struct list_t {
    struct node_t *first;
    type_t type;
    size_t size;
} list_t;

typedef struct queue_control_t {
    list_t queue;
    pthread_mutex_t mtx;
    pthread_cond_t cond;
} queue_control_t;

void init_queue(queue_control_t *queue, type_t type);

void print_queue(const queue_control_t *queue);

void push_queue(queue_control_t *queue, vehicle_control_t *vehicle);

void del_queue(queue_control_t *queue, const vehicle_control_t *vehicle);

vehicle_control_t * pop_queue(queue_control_t *queue);

void exit_wait_queue(queue_control_t *queue, vehicle_control_t *vehicle);
void entry_wait_queue(queue_control_t *queue, vehicle_control_t *vehicle);

void notify_queue(queue_control_t *queue);