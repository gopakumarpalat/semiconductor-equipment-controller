/**
 * @file logger.h
 *
 * @brief Simple logging interface for the equipment control system.
 *
 * @details
 * Provides a reusable logging interface for recording
 * application events, warnings, errors, and informational
 * messages.
 */

#ifndef LOGGER_H
#define LOGGER_H


/**
 * @brief Logging severity levels.
 */
typedef enum
{
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR

} LogLevel;


/**
 * @brief Initialize the logging system.
 *
 * Opens the specified log file for writing/appending.
 *
 * @param filename Path to the log file.
 *
 * @return 0  if initialization is successful.
 * @return -1 if initialization fails.
 */
int logger_init(const char *filename);


/**
 * @brief Write a message to the log.
 *
 * @param level   Severity level of the message.
 * @param message Message to be written to the log.
 */
void logger_log( LogLevel level, const char *message);


/**
 * @brief Close the logging system.
 *
 * Flushes and closes the log file.
 */
void logger_shutdown(void);


#endif