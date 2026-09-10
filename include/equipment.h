#ifndef EQUIPMENT_H
#define EQUIPMENT_H

typedef enum
{
    EQUIPMENT_IDLE,
    EQUIPMENT_READY,
    EQUIPMENT_RUNNING
} EquipmentState;

typedef enum
{
    EQUIPMENT_CMD_INIT,
    EQUIPMENT_CMD_START,
    EQUIPMENT_CMD_STOP
} EquipmentCommand;

typedef struct 
{
    int id;
    char name[32];
    EquipmentState state;
}Equipment;

void equipment_create( Equipment *equipment, int id, const char *name);


void equipment_init(Equipment *equipment);
void equipment_start(Equipment *equipment);
void equipment_stop(Equipment *equipment);
void equipment_print_state(const Equipment *equipment);

#endif