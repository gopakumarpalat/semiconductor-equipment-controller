#include<stdio.h>
#include<string.h>
#include "equipment.h"

/* 
   Initially equipment_init(),equipment_start(), equipment_stop() directly changes the current_state.
   Now we write equipment_transition(), so we have one central location controlling state transitions.
*/
static int equipment_transition(Equipment *equipment, EquipmentCommand command)
{
    switch (equipment->state)
    {
        case EQUIPMENT_IDLE:

            if (command == EQUIPMENT_CMD_INIT)
            {
                equipment->state = EQUIPMENT_READY;
                return 0;
            }

            break;

        case EQUIPMENT_READY:

            if (command == EQUIPMENT_CMD_START)
            {
                equipment->state = EQUIPMENT_RUNNING;
                return 0;
            }

            break;

        case EQUIPMENT_RUNNING:

            if (command == EQUIPMENT_CMD_STOP)
            {
                equipment->state = EQUIPMENT_READY;
                return 0;
            }

            break;

        default:
            break;
    }

    return -1;
}

void equipment_init(Equipment *equipment)
{
    if (equipment_transition(equipment, EQUIPMENT_CMD_INIT) == 0)
    {
        printf("Equipment initialized(IDLE -> READY).\n");
    }
    else
    {
        printf("Cannot initialize equipment.\n");
    }
}

void equipment_start(Equipment *equipment)
{
    if (equipment_transition(equipment, EQUIPMENT_CMD_START) == 0)
    {
        printf("Equipment started(READY -> RUNNING).\n");
    }
    else
    {
        printf("Cannot start equipment from current state.\n");
    }
}

void equipment_stop(Equipment *equipment)
{
    if (equipment_transition(equipment, EQUIPMENT_CMD_STOP) == 0)
    {
        printf("Equipment stopped(RUNNING -> READY).\n");
    }
    else
    {
        printf("Cannot stop equipment from current state.\n");
    }
}

void equipment_print_state(const Equipment *equipment)
{
    switch (equipment->state)
    {
        case EQUIPMENT_IDLE:
            printf("Equipment State: IDLE\n");
            break;

        case EQUIPMENT_READY:
            printf("Equipment State: READY\n");
            break;

        case EQUIPMENT_RUNNING:
            printf("Equipment State: RUNNING\n");
            break;

        default:
            printf("Equipment State: UNKNOWN\n");
            break;
    }
}

void equipment_create(Equipment *equipment, int id, const char *name)
{
    equipment->id = id;

    strcpy(equipment->name, name);

    equipment->state = EQUIPMENT_IDLE;
}