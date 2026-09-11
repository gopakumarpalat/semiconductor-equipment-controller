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

/* Forward declaration */
typedef struct Equipment Equipment;
typedef void (*EquipmentStartCallback)(Equipment *);
typedef void (*EquipmentStopCallback)(Equipment *);
typedef void (*EquipmentAlarmCallback)(Equipment *, const char *);

/* Equipment structure */
struct Equipment
{
    int id;
    char name[32];
    EquipmentState state;
    EquipmentStartCallback start_callback;
    EquipmentStopCallback stop_callback;
    EquipmentAlarmCallback alarm_callback;
};

/* Functions */
void equipment_create( Equipment *equipment, int id, const char *name);
void equipment_init(Equipment *equipment);
void equipment_start(Equipment *equipment);
void equipment_stop(Equipment *equipment);
void equipment_print_state(const Equipment *equipment);


/* Callback setters */
void equipment_set_start_callback( Equipment *equipment, void(*callback)(Equipment *));
void equipment_set_stop_callback( Equipment *equipment, void(*callback)(Equipment *));
void equipment_set_alarm_callback( Equipment *equipment, void (*callback)(Equipment *, const char *));
void equipment_raise_alarm( Equipment *equipment, const char *message);

#endif