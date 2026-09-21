#ifndef EQUIPMENT_CONFIG_H
#define EQUIPMENT_CONFIG_H

/**
 * @brief Equipment configuration.
 *
 * Stores configuration values loaded from the equipment
 * configuration file.
 */
typedef struct
{
    int equipment_id;
    char equipment_name[32];
    int tcp_port;
    char server_ip[64];
    char log_file[128];

    /*
     * Configuration presence flags.
     */
    int has_equipment_id;
    int has_equipment_name;
    int has_tcp_port;
    int has_server_ip;
    int has_log_file;

} EquipmentConfig;


/**
 * @brief Load equipment configuration from a file.
 *
 * @param filename Path to the configuration file.
 * @param config   Pointer to the configuration structure.
 *
 * @return 0 if the configuration was loaded successfully.
 * @return -1 if the configuration file could not be opened.
 */
int equipment_config_load( const char *filename, EquipmentConfig *config );


/**
 * @brief Validate the loaded equipment configuration.
 *
 * Checks whether all required configuration values are
 * within their valid ranges.
 *
 * @param config Pointer to the configuration structure.
 *
 * @return 0 if the configuration is valid.
 * @return -1 if the configuration is invalid.
 */
int equipment_config_validate( const EquipmentConfig *config );

#endif