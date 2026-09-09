#ifndef EQUIPMENT_H
#define EQUIPMENT_H

typedef enum
{
    EQUIPMENT_IDLE,
    EQUIPMENT_READY,
    EQUIPMENT_RUNNING
} EquipmentState;

void equipment_init(void);
void equipment_start(void);
void equipment_stop(void);
void equipment_print_state(void);

#endif