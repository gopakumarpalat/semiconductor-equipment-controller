/**
 * @file config_test.c
 * @brief Test program for the equipment configuration loader.
 */

#include <stdio.h>

#include "equipment_config.h"


int main(void)
{
    EquipmentConfig config = {0};

    /*
     * Load configuration from the configuration file.
     */
    if (equipment_config_load( "config/equipment.conf", &config) != 0)
    {
        printf("Failed to load configuration.\n");
        return 1;
    }

    /*
     * Config validation check
     */
    if (equipment_config_validate(&config) != 0)
    {
        printf("Configuration validation failed.\n");
        return 1;
    }

    printf("Configuration validation successful.\n");

    /*
     * Display the loaded configuration.
     */
    printf("=================================\n");
    printf(" Equipment Configuration\n");
    printf("=================================\n");

    printf("Equipment ID   : %d\n", config.equipment_id);
    printf("Equipment Name : %s\n", config.equipment_name);
    printf("TCP Port       : %d\n", config.tcp_port);

    printf("\nConfiguration Presence:\n");

    printf( "EQUIPMENT_ID   : %s\n", config.has_equipment_id ? "PRESENT" : "MISSING" );

    printf( "EQUIPMENT_NAME : %s\n", config.has_equipment_name ? "PRESENT" : "MISSING" );

    printf( "TCP_PORT       : %s\n", config.has_tcp_port ? "PRESENT" : "MISSING" );

    return 0;
}