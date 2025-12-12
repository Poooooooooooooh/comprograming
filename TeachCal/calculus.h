#ifndef CALCULUS_H
#define CALCULUS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#endif

/* Constants */
#define MAX_QUESTIONS 64
#define MAX_TEXT 512

/* Data structures */
typedef struct {
    char prompt[MAX_TEXT];
    char options[4][MAX_TEXT];
    int correctIndex; /* 0..3 */
    char explanation[MAX_TEXT]; /* Explanation for the answer */
} Question;

typedef struct {
    char code[8]; /* e.g., LIM, DER, APP */
    char name[128]; /* Display name */
    char quiz_file[64];
} Chapter;

/* External declarations */
extern Chapter CHAPTERS[];
extern const int CHAPTER_COUNT;

/* Function declarations */
void trim_newline(char *s);
void read_line(char *buf, size_t sz);
void clear_screen(void);
void setup_console_utf8(void);
int load_quiz_from_file(const char *path, Question *out, int max_out);
int fallback_quiz(const Chapter *chap, Question *out, int max_out);
void shuffle_indices(int *idx, int n);
void save_progress(const char *username, const char *chap_code, int score, int total);
void run_learn_mode(const Chapter *chap);
void run_test_mode(const Chapter *chap, const char *username);
int chapter_menu(const Chapter *chap, const char *username, int mode);
void main_menu(const char *username);

#endif /* CALCULUS_H */

