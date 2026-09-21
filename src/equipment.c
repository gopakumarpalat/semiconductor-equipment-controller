/**
 * @file equipment.c
 * @brief Implementation of the semiconductor equipment control framework.
 *
 * @details
 * This source file implements the equipment state machine, equipment
 * lifecycle functions, event callback mechanism, and equipment event
 * notification functions.
 *
 * The equipment state transitions are protected using a pthread mutex
 * so that multiple threads can safely access and modify equipment state.
 *
 * Supported states:
 * - IDLE
 * - READY
 * - RUNNING
 *
 * Supported commands:
 * - INIT
 * - START
 * - STOP
 *
 * Supported events:
 * - STARTED
 * - STOPPED
 * - ALARM
 *
 * @note This implementation uses POSIX pthread APIs and is intended
 *       to be built and executed in a Linux/WSL environment.
 *
 * @author Gopakumar Palat
 * @date 2026-09-18
 */

#include<stdio.h>
#include<string.h>
#include "equipment.h"

/* ================================================================
 * Private Functions
 * ================================================================ */

/**
 * @brief Perform a state transition for the equipment.
 *
 * This is the central state-machine function. It validates the
 * requested command against the current equipment state and changes
 * the state only when the transition is valid.
 *
 * Valid transitions:
 *
 *     IDLE    + INIT  -> READY
 *     READY   + START -> RUNNING
 *     RUNNING + STOP  -> READY
 *
 * @param equipment Pointer to the equipment instance.
 * @param command   Command requesting a state transition.
 *
 * @return 0 if the transition was successful.
 * @return -1 if the transition is invalid.
 *
 * @note This function is private to this source file and is therefore
 *       declared as static.
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


/* ================================================================
 * Equipment State Control
 * ================================================================ */

/**
 * @brief Initialize the equipment.
 *
 * Attempts to transition the equipment from IDLE to READY.
 *
 * @param equipment Pointer to the equipment instance.
 */
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


/**
 * @brief Start the equipment.
 *
 * Attempts to transition the equipment from READY to RUNNING.
 * If successful, a STARTED event is generated.
 *
 * @param equipment Pointer to the equipment instance.
 *
 * @note The event notification is performed after releasing the
 *       equipment mutex to avoid holding the mutex while executing
 *       callback code.
 */
void equipment_start(Equipment *equipment)
{
    pthread_mutex_lock(&equipment->mutex);

    int result = equipment_transition( equipment, EQUIPMENT_CMD_START );

    pthread_mutex_unlock(&equipment->mutex);

    if (result == 0)
    {
        printf("Equipment started(READY -> RUNNING).\n");

        /*
         * Notify the registered event callback AFTER releasing
         * the equipment mutex.
         */
        equipment_notify_event( equipment, EQUIPMENT_EVENT_STARTED, "Equipment started successfully");
    }
    else
    {
        printf("Cannot start equipment from current state.\n");
    }
}



/**
 * @brief Stop the equipment.
 *
 * Attempts to transition the equipment from RUNNING to READY.
 * If successful, a STOPPED event is generated.
 *
 * @param equipment Pointer to the equipment instance.
 *
 * @note The event notification is performed after releasing the
 *       equipment mutex to avoid holding the mutex while executing
 *       callback code.
 */
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
         * Notify the registered event callback AFTER releasing
         * the equipment mutex.
         */
        equipment_notify_event( equipment, EQUIPMENT_EVENT_STOPPED, "Equipment stopped successfully");
    }
    else
    {
        printf("Cannot stop equipment from current state.\n");
    }
}


/**
 * @brief Print the current equipment state.
 *
 * @param equipment Pointer to the equipment instance.
 */
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



/* ================================================================
 * Equipment Lifecycle
 * ================================================================ */

/**
 * @brief Create and initialize an equipment instance.
 *
 * Initializes the equipment ID, name, initial state, event callback,
 * mutex, and internal counter.
 *
 * @param equipment Pointer to the equipment instance.
 * @param id        Unique equipment identifier.
 * @param name      Equipment name.
 *
 * @note The initial equipment state is EQUIPMENT_IDLE.
 */
void equipment_create(Equipment *equipment, int id, const char *name)
{
    equipment->id = id;

    strcpy(equipment->name, name);

    equipment->state = EQUIPMENT_IDLE;

    equipment->event_callback = NULL;

    pthread_mutex_init( &equipment->mutex, NULL);

    equipment->counter = 0;
}


/**
 * @brief Destroy an equipment instance.
 *
 * Releases resources associated with the equipment mutex.
 *
 * @param equipment Pointer to the equipment instance.
 */
void equipment_destroy(Equipment *equipment)
{
    pthread_mutex_destroy(&equipment->mutex);
}


/* ================================================================
 * Event Callback Management
 * ================================================================ */

/**
 * @brief Register a generic event callback.
 *
 * The registered callback is invoked whenever an equipment event
 * is notified using equipment_notify_event().
 *
 * @param equipment Pointer to the equipment instance.
 * @param callback  Generic event callback function.
 */
void equipment_set_event_callback( Equipment *equipment, EquipmentEventCallback callback)
{
    equipment->event_callback = callback;
}


/**
 * @brief Raise an equipment alarm event.
 *
 * Generates an EQUIPMENT_EVENT_ALARM event and sends the supplied
 * message to the registered event callback.
 *
 * @param equipment Pointer to the equipment instance.
 * @param message   Alarm message describing the condition.
 */
void equipment_raise_alarm( Equipment *equipment, const char *message)
{
    equipment_notify_event( equipment, EQUIPMENT_EVENT_ALARM, message );
}


/**
 * @brief Get the string representation of an equipment event.
 *
 * @param event Equipment event type.
 *
 * @return String representation of the event.
 */
const char *equipment_event_name(EquipmentEvent event)
{
    switch (event)
    {
        case EQUIPMENT_EVENT_STARTED:
            return "STARTED";

        case EQUIPMENT_EVENT_STOPPED:
            return "STOPPED";

        case EQUIPMENT_EVENT_ALARM:
            return "ALARM";

        default:
            return "UNKNOWN";
    }
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