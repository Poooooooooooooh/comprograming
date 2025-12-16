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

/* Console UTF-8 setup */
void setup_console_utf8(void) {
#ifdef _WIN32
    /* Set UTF-8 code pages */
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    
    /* Enable virtual terminal processing for better Unicode support (Windows 10+) */
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode); /* Ignore error if not supported */
        }
    }
    
    /* Try to set a font that supports mathematical symbols */
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
        /* Use CONSOLE_FONT_INFOEX if available (Windows Vista+) */
        typedef struct _CONSOLE_FONT_INFOEX_WRAP {
            ULONG cbSize;
            DWORD nFont;
            COORD dwFontSize;
            UINT FontFamily;
            UINT FontWeight;
            WCHAR FaceName[LF_FACESIZE];
        } CONSOLE_FONT_INFOEX_WRAP;
        
        typedef BOOL (WINAPI *GetCurrentConsoleFontExProc)(HANDLE, BOOL, void*);
        typedef BOOL (WINAPI *SetCurrentConsoleFontExProc)(HANDLE, BOOL, void*);
        
        HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
        if (hKernel32) {
            GetCurrentConsoleFontExProc pGetFont = (GetCurrentConsoleFontExProc)
                GetProcAddress(hKernel32, "GetCurrentConsoleFontEx");
            SetCurrentConsoleFontExProc pSetFont = (SetCurrentConsoleFontExProc)
                GetProcAddress(hKernel32, "SetCurrentConsoleFontEx");
            
            if (pGetFont && pSetFont) {
                CONSOLE_FONT_INFOEX_WRAP cfi;
                memset(&cfi, 0, sizeof(cfi));
                cfi.cbSize = sizeof(cfi);
                if (pGetFont(hConsole, FALSE, &cfi)) {
                    /* Try to use Consolas or a font that supports Unicode */
                    #ifdef __MINGW32__
                        wcscpy(cfi.FaceName, L"Consolas");
                    #else
                        wcscpy_s(cfi.FaceName, LF_FACESIZE, L"Consolas");
                    #endif
                    cfi.dwFontSize.X = 0;
                    cfi.dwFontSize.Y = 16;
                    cfi.FontFamily = FF_DONTCARE;
                    cfi.FontWeight = FW_NORMAL;
                    pSetFont(hConsole, FALSE, &cfi); /* Ignore error if fails */
                }
            }
        }
    }
    
    /* Set console buffer size for better display */
    HANDLE hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsoleOutput != INVALID_HANDLE_VALUE) {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(hConsoleOutput, &csbi)) {
            COORD newSize;
            newSize.X = csbi.dwSize.X;
            newSize.Y = 3000; /* Increase buffer for scrolling */
            SetConsoleScreenBufferSize(hConsoleOutput, newSize);
        }
    }
#endif
}

