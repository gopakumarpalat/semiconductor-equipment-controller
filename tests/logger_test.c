/**
 * @file logger_test.c
 *
 * @brief Test program for the logging module.
 */

#include <stdio.h>

#include "logger.h"


int main(void)
{
    /*
     * Initialize logger.
     */
    if (logger_init("logs/equipment.log") != 0)
    {
        printf("Failed to initialize logger.\n");
        return 1;
    }

    printf("Logger initialized successfully.\n");


    /*
     * Test different log levels.
     */
    logger_log( LOG_INFO, "Equipment server started." );

    logger_log( LOG_WARNING, "Equipment temperature is approaching the limit." );

    logger_log( LOG_ERROR, "Failed to communicate with equipment." );


    /*
     * Close logger.
     */
    logger_shutdown();

    printf("Logger test completed successfully.\n");

    return 0;
}