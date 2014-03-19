#ifndef LOG_H
#define LOG_H

void log_printf(const char *fmt, ...);

void log_init(const char *name, const char *path);
void log_quit(void);

#endif
