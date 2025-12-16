#include "quiz_engine.h"
#include "data_load.h"
#include "utils.h"
#include "logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 
 * quiz_engine.c
 * -------------
 * This module is responsible only for *running* individual quizzes
 * in Learn or Test mode for a single chapter.
 *
 * It does NOT know about the main menu, chapter selection, or
 * score history. Those higher-level flows are handled by main.c.
 */

/* Learn mode: Show immediate feedback with explanations (no score recording) */
void run_learn_mode(const Chapter *chap) {
    clear_screen();
    printf("=== Learn Mode: %s ===\n\n", chap->name);
    printf("Type 'q' to quit and return to menu at any time\n\n");

    Question all[MAX_QUESTIONS];
    int total_loaded = 0;
    if (chap->quiz_file[0] != '\0') {
        total_loaded = load_quiz_from_file(chap->quiz_file, all, MAX_QUESTIONS);
    }
    if (total_loaded == 0) {
        total_loaded = fallback_quiz(chap, all, MAX_QUESTIONS);
        printf("(Using sample quiz: %d questions)\n\n", total_loaded);
    }

    /* Ask user for number of questions */
    printf("How many questions would you like? (Press Enter for all %d questions): ", total_loaded);
    char num_input[16];
    read_line(num_input, sizeof(num_input));
    
    int ask_total = total_loaded;
    if (num_input[0] != '\0') {
        int requested = atoi(num_input);
        if (requested > 0 && requested <= total_loaded) {
            ask_total = requested;
        } else if (requested > total_loaded) {
            printf("Only %d questions available. Using all questions.\n", total_loaded);
            printf("Press Enter to continue...");
            char wait[8];
            read_line(wait, sizeof(wait));
        }
    }

    int idx[MAX_QUESTIONS];
    for (int i = 0; i < total_loaded; i++) idx[i] = i;
    shuffle_indices(idx, total_loaded);
    
    for (int k = 0; k < ask_total; k++) {
        int qi = idx[k];
        Question *q = &all[qi];
        clear_screen();
        printf("=== Learn Mode: %s ===\n", chap->name);
        printf("Question %d/%d\n\n", k+1, ask_total);
        printf("%s\n", q->prompt);
        for (int i = 0; i < 4; i++) {
            printf("  %d) %s\n", i+1, q->options[i]);
        }
        printf("\nAnswer (1-4, q=quit): ");
        char inp[16];
        read_line(inp, sizeof(inp));
        
        if (strcmp(inp, "q") == 0 || strcmp(inp, "Q") == 0 || strcmp(inp, "quit") == 0 || strcmp(inp, "exit") == 0 || strcmp(inp, "0") == 0) {
            printf("\nQuit. Questions completed: %d\n", k);
            printf("\nPress Enter to return to menu...");
            char wait[8];
            read_line(wait, sizeof(wait));
            return;
        }
        
        int ans = atoi(inp);
        if (ans < 1 || ans > 4) {
            printf("\nInvalid input. Please try again.\n");
            printf("Press Enter to continue...");
            char wait[8];
            read_line(wait, sizeof(wait));
            k--; /* Repeat this question */
            continue;
        }
        
        int correct = q->correctIndex + 1;
        printf("\n");
        if (ans == correct) {
            printf("Correct! ");
        } else {
            printf("Incorrect. ");
        }
        printf("The correct answer is %d) %s\n", correct, q->options[correct-1]);
        
        /* Show explanation if available */
        if (q->explanation[0] != '\0') {
            printf("\nExplanation: %s\n", q->explanation);
        }
        
        printf("\nPress Enter to continue to next question...");
        char wait[8];
        read_line(wait, sizeof(wait));
    }
    
    clear_screen();
    printf("=== Learn Mode Complete ===\n");
    printf("You have completed all %d questions!\n", ask_total);
    printf("\nPress Enter to return to menu...");
    char wait[8];
    read_line(wait, sizeof(wait));
}

/* Test mode: Collect all answers, then show results and record score */
void run_test_mode(const Chapter *chap, const char *username) {
    clear_screen();
    printf("=== Test Mode: %s ===\n\n", chap->name);
    printf("Type 'q' to quit and return to menu at any time\n");
    printf("You will answer all questions, then see your results.\n\n");

    Question all[MAX_QUESTIONS];
    int total_loaded = 0;
    if (chap->quiz_file[0] != '\0') {
        total_loaded = load_quiz_from_file(chap->quiz_file, all, MAX_QUESTIONS);
    }
    if (total_loaded == 0) {
        total_loaded = fallback_quiz(chap, all, MAX_QUESTIONS);
        printf("(Using sample quiz: %d questions)\n\n", total_loaded);
    }

    /* Ask user for number of questions */
    printf("How many questions would you like? (Press Enter for all %d questions): ", total_loaded);
    char num_input[16];
    read_line(num_input, sizeof(num_input));
    
    int ask_total = total_loaded;
    if (num_input[0] != '\0') {
        int requested = atoi(num_input);
        if (requested > 0 && requested <= total_loaded) {
            ask_total = requested;
        } else if (requested > total_loaded) {
            printf("Only %d questions available. Using all questions.\n", total_loaded);
            printf("Press Enter to continue...");
            char wait[8];
            read_line(wait, sizeof(wait));
        }
    }

    int idx[MAX_QUESTIONS];
    for (int i = 0; i < total_loaded; i++) idx[i] = i;
    shuffle_indices(idx, total_loaded);
    int answers[MAX_QUESTIONS] = {0};
    
    /* Collect all answers without showing feedback */
    for (int k = 0; k < ask_total; k++) {
        int qi = idx[k];
        Question *q = &all[qi];
        clear_screen();
        printf("=== Test Mode: %s ===\n", chap->name);
        printf("Question %d/%d\n\n", k+1, ask_total);
        printf("%s\n", q->prompt);
        for (int i = 0; i < 4; i++) {
            printf("  %d) %s\n", i+1, q->options[i]);
        }
        printf("\nAnswer (1-4, q=quit): ");
        char inp[16];
        read_line(inp, sizeof(inp));
        
        if (strcmp(inp, "q") == 0 || strcmp(inp, "Q") == 0 || strcmp(inp, "quit") == 0 || strcmp(inp, "exit") == 0 || strcmp(inp, "0") == 0) {
            printf("\nQuit. Test incomplete.\n");
            printf("\nPress Enter to return to menu...");
            char wait[8];
            read_line(wait, sizeof(wait));
            return;
        }
        
        int ans = atoi(inp);
        if (ans < 1 || ans > 4) {
            printf("\nInvalid input. Please enter 1-4.\n");
            printf("Press Enter to try again...");
            char wait[8];
            read_line(wait, sizeof(wait));
            k--; /* Repeat this question */
            continue;
        }
        
        answers[k] = ans;
    }
    
    /* Calculate score and show results */
    int score = 0;
    clear_screen();
    printf("=== Test Results: %s ===\n\n", chap->name);
    
    for (int k = 0; k < ask_total; k++) {
        int qi = idx[k];
        Question *q = &all[qi];
        int ans = answers[k];
        int correct = q->correctIndex + 1;
        
        printf("Question %d: %s\n", k+1, q->prompt);
        printf("Your answer: %d) %s\n", ans, q->options[ans-1]);
        printf("Correct answer: %d) %s\n", correct, q->options[correct-1]);
        
        if (ans == correct) {
            printf("Status: CORRECT\n");
            score++;
        } else {
            printf("Status: INCORRECT\n");
        }
        
        if (q->explanation[0] != '\0') {
            printf("Explanation: %s\n", q->explanation);
        }
        printf("\n");
    }
    
    double percentage;
    if (ask_total > 0) {
        percentage = (double)score / ask_total * 100.0;
    } else {
        percentage = 0.0;
    }
    printf("=== Final Score ===\n");
    printf("Score: %d/%d\n", score, ask_total);
    printf("Percentage: %.1f%%\n", percentage);
    
    /* Log test results */
    log_test_result(username, chap->code, score, ask_total);
    printf("\nResults saved to logs/quiz_score.log\n");
    printf("\nPress Enter to return to menu...");
    char wait[8];
    read_line(wait, sizeof(wait));
}

