#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#include <wchar.h>
/* Define Windows version requirements for newer console features */
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601 /* Windows 7+ */
#endif
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

/* Utility: trim newline */
void trim_newline(char *s) {
    if (!s) return;
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r')) {
        s[n-1] = '\0';
        n--;
    }
}

/* Utility: read a line from stdin safely */
void read_line(char *buf, size_t sz) {
    if (!fgets(buf, (int)sz, stdin)) {
        buf[0] = '\0';
        clearerr(stdin);
    } else {
        trim_newline(buf);
    }
}

/* Utility: clear screen (Windows/Linux) */
void clear_screen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/* Console UTF-8 setup (simplified so beginners can understand it more easily) */
void setup_console_utf8(void) {
#ifdef _WIN32
    /* Just set the console code page to UTF-8.
       This is enough for our simple quiz program. */
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif
}

