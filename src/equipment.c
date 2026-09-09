#include<stdio.h>
#include "equipment.h"

static EquipmentState current_state = EQUIPMENT_IDLE;


/* 
   Initially equipment_init(),equipment_start(), equipment_stop() directly changes the current_state.
   Now we write equipment_transition(), so we have one central location controlling state transitions.
*/
static int equipment_transition(EquipmentCommand command)
{
    switch (current_state)
    {
        case EQUIPMENT_IDLE:

            if (command == EQUIPMENT_CMD_INIT)
            {
                current_state = EQUIPMENT_READY;
                return 0;
            }

            break;

        case EQUIPMENT_READY:

            if (command == EQUIPMENT_CMD_START)
            {
                current_state = EQUIPMENT_RUNNING;
                return 0;
            }

            break;

        case EQUIPMENT_RUNNING:

            if (command == EQUIPMENT_CMD_STOP)
            {
                current_state = EQUIPMENT_READY;
                return 0;
            }

            break;

        default:
            break;
    }

    return -1;
}

void equipment_init()
{
    if (equipment_transition(EQUIPMENT_CMD_INIT) == 0)
    {
        printf("Equipment initialized.\n");
    }
    else
    {
        printf("Cannot initialize equipment.\n");
    }
}

void equipment_start()
{
    if (equipment_transition(EQUIPMENT_CMD_START) == 0)
    {
        printf("Equipment started.\n");
    }
    else
    {
        printf("Cannot start equipment from current state.\n");
    }

}

void equipment_stop()
{
    if (equipment_transition(EQUIPMENT_CMD_STOP) == 0)
    {
        printf("Equipment stopped.\n");
    }
    else
    {
        printf("Cannot stop equipment from current state.\n");
    }
}

void equipment_print_state()
{
    switch (current_state)
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