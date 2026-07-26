#ifndef __LOGGER_H
#define __LOGGER_H

#include <stdio.h>


typedef enum {
	LOG_EVENT_ERROR = 0,
	LOG_EVENT_WARN,
	LOG_EVENT_INFO,
	LOG_EVENT_DEBUG,
} log_event_t;

void log_init(const char *log_file, log_event_t min_level, int max_size);
void log_add_output(FILE *fp);
void log_write(log_event_t level, const char *module, const char *fmt, ...);
void logger_shutdown(void);

#define LOG_ERROR(mod, fmt, ...) log_write(LOG_EVENT_ERROR, mod, fmt, ##__VA_ARGS__)
#define LOG_WARN(mod, fmt, ...) log_write(LOG_EVENT_WARN, mod, fmt, ##__VA_ARGS__)
#define LOG_INFO(mod, fmt, ...) log_write(LOG_EVENT_INFO, mod, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(mod, fmt, ...) log_write(LOG_EVENT_DEBUG, mod, fmt, ##__VA_ARGS__)

#endif
