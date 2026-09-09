#include <stdio.h>
#include "equipment.h"

enum EquipmentState
{
    IDLE,
    READY,
    RUNNING
};

int main()
{
    printf("Semiconductor Equipment Controller\n");

    equipment_print_state();

    equipment_init();
    equipment_print_state();

    equipment_start();
    equipment_print_state();

    equipment_stop();
    equipment_print_state();

    return 0;
}