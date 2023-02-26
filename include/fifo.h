#include <types.h>

void print_queue();

void push_queue(vehicle_t *vehicle);

void del_queue(const vehicle_t *vehicle);

vehicle_t * pop_queue();

int16_t is_first_queue(vehicle_t *vehicle);
