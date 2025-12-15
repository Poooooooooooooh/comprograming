#include "quiz_engine.h"
#include "data_load.h"
#include "utils.h"
#include "logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    
    double percentage = (double)score / ask_total * 100.0;
    printf("=== Final Score ===\n");
    printf("Score: %d/%d\n", score, ask_total);
    printf("Percentage: %.1f%%\n", percentage);
    
    /* Log test results */
    log_test_result(username, chap->code, score, ask_total);
    printf("\nResults saved to logging.h\n");
    printf("\nPress Enter to return to menu...");
    char wait[8];
    read_line(wait, sizeof(wait));
}

/* Per-chapter submenu */
int chapter_menu(const Chapter *chap, const char *username, int mode) {
    /* mode: 0 = Learn, 1 = Test */
    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "User: %s | Mode: %s | Chapter: %s", 
             username, mode == 0 ? "Learn" : "Test", chap->name);
    log_event("INFO", log_msg);
    
    if (mode == 0) {
        run_learn_mode(chap);
    } else {
        run_test_mode(chap, username);
    }
    return 0; /* back to main menu */
}

/* Display score history from log file */
void show_score_history(void) {
    clear_screen();
    printf("=== Score History ===\n\n");
    
    FILE *fp = fopen("logs/quiz_score.log", "r");
    if (!fp) {
        printf("No score history found. Take a test to generate scores!\n");
        printf("\nPress Enter to return to menu...");
        char wait[8];
        read_line(wait, sizeof(wait));
        return;
    }
    
    char line[512];
    int count = 0;
    int test_count = 0;
    
    /* Count total lines and test entries */
    while (fgets(line, sizeof(line), fp)) {
        count++;
        if (strstr(line, "[TEST]")) {
            test_count++;
        }
    }
    
    if (test_count == 0) {
        fclose(fp);
        printf("No test scores found. Take a test to generate scores!\n");
        printf("\nPress Enter to return to menu...");
        char wait[8];
        read_line(wait, sizeof(wait));
        return;
    }
    
    /* Rewind and collect all test entries */
    rewind(fp);
    
    /* Allocate array for all test entries */
    char **lines = (char **)malloc(test_count * sizeof(char *));
    if (!lines) {
        fclose(fp);
        printf("Memory allocation error.\n");
        printf("\nPress Enter to return to menu...");
        char wait[8];
        read_line(wait, sizeof(wait));
        return;
    }
    
    for (int i = 0; i < test_count; i++) {
        lines[i] = (char *)malloc(512 * sizeof(char));
        if (!lines[i]) {
            /* Free already allocated */
            for (int j = 0; j < i; j++) free(lines[j]);
            free(lines);
            fclose(fp);
            printf("Memory allocation error.\n");
            printf("\nPress Enter to return to menu...");
            char wait[8];
            read_line(wait, sizeof(wait));
            return;
        }
    }
    
    int line_count = 0;
    
    /* Read all test entries */
    while (fgets(line, sizeof(line), fp) && line_count < test_count) {
        trim_newline(line);
        if (strstr(line, "[TEST]")) {
            strncpy(lines[line_count], line, 511);
            lines[line_count][511] = '\0';
            line_count++;
        }
    }
    
    fclose(fp);
    
    /* Display results */
    printf("Test Results (showing last 50 entries):\n");
    printf("========================================\n\n");
    
    int max_display = 50;
    int start_idx = (line_count > max_display) ? (line_count - max_display) : 0;
    
    /* Display in reverse order (most recent first) */
    for (int i = line_count - 1; i >= start_idx; i--) {
        printf("%s\n", lines[i]);
    }
    
    if (test_count > max_display) {
        printf("\n... (showing last %d of %d test results)\n", max_display, test_count);
    } else {
        printf("\nTotal test results: %d\n", test_count);
    }
    
    /* Free allocated memory */
    for (int i = 0; i < test_count; i++) {
        free(lines[i]);
    }
    free(lines);
    
    printf("\nPress Enter to return to menu...");
    char wait[8];
    read_line(wait, sizeof(wait));
}

/* Main menu - choose mode first, then chapter */
void main_menu(const char *username) {
    while (1) {
        clear_screen();
        printf("=== Calculus 1 Tutor ===\n");
        printf("User: %s\n\n", username);
        printf("Choose mode:\n");
        printf("1) Learn Mode (immediate feedback with explanations, no score)\n");
        printf("2) Test Mode (submit all answers, see results and score)\n");
        printf("3) Exit program\n");
        printf("5) Score History\n");
        printf("Choose: ");
        char inp[16];
        read_line(inp, sizeof(inp));
        int mode_sel = atoi(inp);
        
        if (mode_sel == 3) {
            break; /* exit */
        }
        
        if (mode_sel == 5) {
            show_score_history();
            continue;
        }
        
        if (mode_sel != 1 && mode_sel != 2) {
            continue; /* invalid input */
        }
        
        int mode = mode_sel - 1; /* 0 = Learn, 1 = Test */
        
        /* Choose chapter */
        while (1) {
            clear_screen();
            printf("=== Calculus 1 Tutor ===\n");
            printf("User: %s\n", username);
            printf("Mode: %s\n\n", mode == 0 ? "Learn" : "Test");
            for (int i = 0; i < CHAPTER_COUNT; i++) {
                printf("%d) %s\n", i+1, CHAPTERS[i].name);
            }
            printf("%d) Back to mode selection\n", CHAPTER_COUNT + 1);
            printf("Choose chapter: ");
            read_line(inp, sizeof(inp));
            int chap_sel = atoi(inp);
            
            if (chap_sel == CHAPTER_COUNT + 1) {
                break; /* back to mode selection */
            }
            
            if (chap_sel >= 1 && chap_sel <= CHAPTER_COUNT) {
                chapter_menu(&CHAPTERS[chap_sel-1], username, mode);
            }
        }
    }
}

