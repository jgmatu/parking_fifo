#include <types.h>

void print_fifo();

void push_fifo(vehicle_t *vehicle);

void del_fifo(const vehicle_t *vehicle);

vehicle_t * pop_fifo();

int16_t is_first_fifo(vehicle_t *vehicle);
