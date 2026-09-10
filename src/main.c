#include <stdio.h>
#include <string.h>
#include "equipment.h"

enum EquipmentState
{
    IDLE,
    READY,
    RUNNING
};

int main()
{
    Equipment etch;
    Equipment cvd;

    equipment_create(&etch, 101, "ETCH01");
    equipment_create(&cvd, 102, "CVD01");

    printf("=================================\n");
    printf(" Semiconductor Equipment Control\n");
    printf("=================================\n\n");

    printf("Equipment 1:\n");
    printf("ID   : %d\n", etch.id);
    printf("Name : %s\n", etch.name);

    printf("\nEquipment 2:\n");
    printf("ID   : %d\n", cvd.id);
    printf("Name : %s\n", cvd.name);

    printf("\nStarting ETCH01...\n");

    equipment_init(&etch);
    equipment_start(&etch);

    printf("\nETCH01 state:\n");
    equipment_print_state(&etch);

    printf("\nCVD01 state:\n");
    equipment_print_state(&cvd);

    return 0;
}