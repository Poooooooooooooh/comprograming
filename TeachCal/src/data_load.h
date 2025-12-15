#ifndef DATA_LOAD_H
#define DATA_LOAD_H

#include "data_structs.h"

/* Parse quiz file into Question array; return count loaded */
/* Format: question, 4 options, answer (1-4) */
int load_quiz_from_file(const char *path, Question *out, int max_out);

/* Fallback quiz for each chapter; returns count */
int fallback_quiz(const Chapter *chap, Question *out, int max_out);

/* Shuffle indices */
void shuffle_indices(int *idx, int n);

#endif /* DATA_LOAD_H */

