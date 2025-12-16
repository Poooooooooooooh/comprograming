#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

/* Utility: trim newline */
void trim_newline(char *s);

/* Utility: read a line from stdin safely */
void read_line(char *buf, size_t sz);

/* Utility: clear screen (Windows/Linux) */
void clear_screen(void);

/* Console UTF-8 setup */
void setup_console_utf8(void);

#endif /* UTILS_H */

