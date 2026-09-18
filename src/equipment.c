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
    pthread_mutex_lock(&equipment->mutex);

    int result = equipment_transition( equipment, EQUIPMENT_CMD_INIT );

    pthread_mutex_unlock(&equipment->mutex);

    if (result == 0)
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
    pthread_mutex_lock(&equipment->mutex);

    int result = equipment_transition( equipment, EQUIPMENT_CMD_START );

    pthread_mutex_unlock(&equipment->mutex);

    if (result == 0)
    {
        printf("Equipment started(READY -> RUNNING).\n");

        /*
         * Call callback AFTER releasing mutex.
         */

        equipment_notify_event( equipment, EQUIPMENT_EVENT_STARTED, "Equipment started successfully");
    }
    else
    {
        printf("Cannot start equipment from current state.\n");
    }
}

void equipment_stop(Equipment *equipment)
{
    pthread_mutex_lock(&equipment->mutex);

    int result =
        equipment_transition(
            equipment,
            EQUIPMENT_CMD_STOP
        );

    pthread_mutex_unlock(&equipment->mutex);

    if (result == 0)
    {
        printf("Equipment stopped(RUNNING -> READY).\n");

        /*
         * Call callback AFTER releasing mutex.
         */

        equipment_notify_event( equipment, EQUIPMENT_EVENT_STOPPED, "Equipment stopped successfully");
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

    equipment->event_callback = NULL;

    pthread_mutex_init( &equipment->mutex, NULL);

    equipment->counter = 0;
}

void equipment_destroy(Equipment *equipment)
{
    pthread_mutex_destroy(&equipment->mutex);
}


void equipment_set_event_callback( Equipment *equipment, EquipmentEventCallback callback)
{
    equipment->event_callback = callback;
}

void equipment_raise_alarm( Equipment *equipment, const char *message)
{
    equipment_notify_event( equipment, EQUIPMENT_EVENT_ALARM, message );
}


/**
 * @brief Notify the registered event callback about an equipment event.
 *
 * This function checks whether a generic event callback has been
 * registered for the equipment. If registered, it invokes the callback
 * and passes the equipment instance, event type, and optional message.
 *
 * @param equipment Pointer to the equipment instance.
 * @param event     Event type to be notified.
 * @param message   Optional message associated with the event.
 *
 * @note The callback is invoked only when a valid event callback
 *       has been registered using equipment_set_event_callback().
 */
void equipment_notify_event( Equipment *equipment, EquipmentEvent event, const char *message)
{
    if (equipment->event_callback != NULL)
    {
        equipment->event_callback( equipment, event, message);
    }
}