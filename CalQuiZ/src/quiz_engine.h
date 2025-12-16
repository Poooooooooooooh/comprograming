#ifndef QUIZ_ENGINE_H
#define QUIZ_ENGINE_H

#include "data_structs.h"

/* Learn mode: Show immediate feedback with explanations (no score recording) */
void run_learn_mode(const Chapter *chap);

/* Test mode: Collect all answers, then show results and record score */
void run_test_mode(const Chapter *chap, const char *username);

#endif /* QUIZ_ENGINE_H */

