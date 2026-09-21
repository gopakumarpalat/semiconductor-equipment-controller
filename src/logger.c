/**
 * @file logger.c
 *
 * @brief Implementation of the equipment control logging system.
 *
 * @details
 * Provides functions for initializing the logger, writing log
 * messages with timestamps and severity levels, and shutting
 * down the logger.
 */

#include <stdio.h>
#include <time.h>
#include <pthread.h>

#include "logger.h"


/* =========================================================================
 * Private Variables
 * ========================================================================= */

/**
 * @brief Log file used by the logging system.
 */
static FILE *log_file = NULL;


/**
 * @brief Mutex protecting the log file.
 *
 * Multiple worker threads may write log messages at the same time.
 * The mutex ensures that log messages are not mixed together.
 */
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;


/* =========================================================================
 * Private Functions
 * ========================================================================= */

/**
 * @brief Convert a log level to a readable string.
 *
 * @param level Log severity level.
 *
 * @return String representation of the log level.
 */
static const char *logger_level_name(LogLevel level)
{
    switch (level)
    {
        case LOG_INFO:
            return "INFO";

        case LOG_WARNING:
            return "WARNING";

        case LOG_ERROR:
            return "ERROR";

        default:
            return "UNKNOWN";
    }
}


/* =========================================================================
 * Public Functions
 * ========================================================================= */

/**
 * @brief Initialize the logging system.
 *
 * @param filename Path to the log file.
 *
 * @return 0 if successful.
 * @return -1 if initialization fails.
 */
int logger_init(const char *filename)
{
    if (filename == NULL)
    {
        return -1;
    }

    /*
     * Open the log file in append mode.
     *
     * Existing log entries are preserved.
     */
    log_file = fopen(filename, "a");

    if (log_file == NULL)
    {
        perror("logger: fopen");
        return -1;
    }

    return 0;
}


/**
 * @brief Write a message to the log file.
 *
 * @param level   Severity level.
 * @param message Message to be logged.
 */
void logger_log( LogLevel level, const char *message)
{
    if (log_file == NULL || message == NULL)
    {
        return;
    }

    /*
     * Get the current time.
     */
    time_t current_time = time(NULL);

    struct tm time_info;

    /*
     * Convert the current time to local time.
     */
    localtime_r(&current_time, &time_info);

    /*
     * Protect the log file from simultaneous writes
     * by multiple threads.
     */
    pthread_mutex_lock(&log_mutex);

    fprintf(
        log_file,
        "[%04d-%02d-%02d %02d:%02d:%02d] [%s] %s\n",
        time_info.tm_year + 1900,
        time_info.tm_mon + 1,
        time_info.tm_mday,
        time_info.tm_hour,
        time_info.tm_min,
        time_info.tm_sec,
        logger_level_name(level),
        message
    );

    /*
     * Flush immediately so that important log messages
     * are written to disk without waiting for the buffer.
     */
    fflush(log_file);

    pthread_mutex_unlock(&log_mutex);
}


/**
 * @brief Shut down the logging system.
 */
void logger_shutdown(void)
{
    if (log_file == NULL)
    {
        return;
    }

    pthread_mutex_lock(&log_mutex);

    fclose(log_file);

    log_file = NULL;

    pthread_mutex_unlock(&log_mutex);
}