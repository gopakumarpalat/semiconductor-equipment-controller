/**
 * @file equipment_config.c
 *
 * @brief Equipment configuration file loader and validator.
 *
 * @details
 * This module loads equipment configuration values from a
 * text-based configuration file and validates the loaded values.
 *
 * Supported configuration parameters:
 *
 *     EQUIPMENT_ID
 *     EQUIPMENT_NAME
 *     TCP_PORT
 *     SERVER_IP
 *
 * Example configuration:
 *
 *     EQUIPMENT_ID=101
 *     EQUIPMENT_NAME=ETCH01
 *     TCP_PORT=5000
 *     SERVER_IP=127.0.0.1
 *
 * @author Gopakumar Palat
 * @date 2026-09-21
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "equipment_config.h"


/* =========================================================================
 * Configuration Loading
 * ========================================================================= */

/**
 * @brief Load equipment configuration from a file.
 *
 * @param filename Configuration file path.
 * @param config  Pointer to the configuration structure.
 *
 * @return 0  Configuration file loaded successfully.
 * @return -1 If the file cannot be opened or an invalid argument is given.
 */
int equipment_config_load( const char *filename, EquipmentConfig *config)
{
    FILE *file;

    /*
     * Validate function arguments.
     */
    if (filename == NULL || config == NULL)
    {
        printf("Configuration error: Invalid argument.\n");
        return -1;
    }

    /*
     * Initialize configuration values.
     *
     * TCP port has a default value of 5000.
     * Other values do not have defaults because they are required.
     */
    config->equipment_id = 0;
    config->equipment_name[0] = '\0';
    config->tcp_port = 5000;
    config->server_ip[0] = '\0';

    /*
     * Reset configuration presence flags.
     */
    config->has_equipment_id = 0;
    config->has_equipment_name = 0;
    config->has_tcp_port = 0;
    config->has_server_ip = 0;

    /*
     * Open configuration file.
     */
    file = fopen(filename, "r");

    if (file == NULL)
    {
        perror("Configuration file");
        return -1;
    }

    /*
     * Read configuration file line by line.
     */
    char line[128];

    while (fgets(line, sizeof(line), file) != NULL)
    {
        char key[64];
        char value[64];

        /*
         * Ignore empty lines.
         */
        if (line[0] == '\n' || line[0] == '\0')
        {
            continue;
        }

        /*
         * Parse KEY=VALUE format.
         */
        if (sscanf( line, "%63[^=]=%63[^\n]", key, value) != 2)
        {
            printf( "Invalid configuration line: %s", line );

            continue;
        }

        /*
         * Remove Windows carriage return if present.
         *
         * This allows the configuration file to work correctly
         * when edited on Windows and executed in WSL/Linux.
         */
        value[strcspn(value, "\r")] = '\0';


        /* -------------------------------------------------------------
         * EQUIPMENT_ID
         * ------------------------------------------------------------- */

        if (strcmp(key, "EQUIPMENT_ID") == 0)
        {
            config->equipment_id = atoi(value);
            config->has_equipment_id = 1;
        }


        /* -------------------------------------------------------------
         * EQUIPMENT_NAME
         * ------------------------------------------------------------- */

        else if (strcmp(key, "EQUIPMENT_NAME") == 0)
        {
            strncpy( config->equipment_name, value, sizeof(config->equipment_name) - 1 );

            config->equipment_name[ sizeof(config->equipment_name) - 1 ] = '\0';

            config->has_equipment_name = 1;
        }


        /* -------------------------------------------------------------
         * TCP_PORT
         * ------------------------------------------------------------- */

        else if (strcmp(key, "TCP_PORT") == 0)
        {
            config->tcp_port = atoi(value);
            config->has_tcp_port = 1;
        }


        /* -------------------------------------------------------------
         * SERVER_IP
         * ------------------------------------------------------------- */

        else if (strcmp(key, "SERVER_IP") == 0)
        {
            strncpy( config->server_ip, value, sizeof(config->server_ip) - 1 );

            config->server_ip[ sizeof(config->server_ip) - 1] = '\0';

            config->has_server_ip = 1;
        }


        /* -------------------------------------------------------------
         * Unknown configuration key
         * ------------------------------------------------------------- */

        else
        {
            printf("Unknown configuration key: %s\n", key);
        }
    }

    fclose(file);

    return 0;
}


/* =========================================================================
 * Configuration Validation
 * ========================================================================= */

/**
 * @brief Validate loaded equipment configuration.
 *
 * Required parameters:
 *     EQUIPMENT_ID
 *     EQUIPMENT_NAME
 *     SERVER_IP
 *
 * Optional parameter:
 *     TCP_PORT
 *
 * Default:
 *     TCP_PORT = 5000
 *
 * @param config Pointer to the configuration structure.
 *
 * @return 0  Configuration is valid.
 * @return -1 Configuration is invalid.
 */
int equipment_config_validate(
    const EquipmentConfig *config)
{
    if (config == NULL)
    {
        printf( "Configuration error: NULL configuration.\n" );

        return -1;
    }


    /* ---------------------------------------------------------------------
     * EQUIPMENT_ID
     * --------------------------------------------------------------------- */

    if (config->has_equipment_id == 0)
    {
        printf( "Configuration error: EQUIPMENT_ID is missing.\n" );

        return -1;
    }

    if (config->equipment_id <= 0)
    {
        printf( "Configuration error: Invalid equipment ID: %d\n", config->equipment_id );

        return -1;
    }


    /* ---------------------------------------------------------------------
     * EQUIPMENT_NAME
     * --------------------------------------------------------------------- */

    if (config->has_equipment_name == 0)
    {
        printf( "Configuration error: EQUIPMENT_NAME is missing.\n" );

        return -1;
    }

    if (config->equipment_name[0] == '\0')
    {
        printf( "Configuration error: Equipment name is empty.\n" );

        return -1;
    }


    /* ---------------------------------------------------------------------
     * TCP_PORT
     *
     * If TCP_PORT is missing, the default value 5000 is used.
     * If it is present, it must be within the valid TCP port range.
     * --------------------------------------------------------------------- */

    if (config->tcp_port < 1 ||
        config->tcp_port > 65535)
    {
        printf("Configuration error: Invalid TCP port: %d\n", config->tcp_port);

        return -1;
    }


    /* ---------------------------------------------------------------------
     * SERVER_IP
     * --------------------------------------------------------------------- */

    if (config->has_server_ip == 0)
    {
        printf("Configuration error: SERVER_IP is missing.\n" );

        return -1;
    }

    if (config->server_ip[0] == '\0')
    {
        printf("Configuration error: SERVER_IP is empty.\n" );

        return -1;
    }

    return 0;
}