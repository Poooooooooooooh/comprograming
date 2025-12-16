#ifndef DATA_STRUCTS_H
#define DATA_STRUCTS_H

#define MAX_QUESTIONS 64
#define MAX_TEXT 512

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

/* Chapter definitions */
extern Chapter CHAPTERS[];
extern const int CHAPTER_COUNT;

#endif /* DATA_STRUCTS_H */

