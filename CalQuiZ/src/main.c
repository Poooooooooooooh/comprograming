/*
 * main.c
 * -------
 * Core engine / controller for the CalQuiZ system.
 *
 * Responsibilities:
 *  - Handle overall program flow (startup and shutdown).
 *  - Display the main menu and chapter selection menus.
 *  - Run Learn and Test modes for selected chapters
 *    by calling the quiz runner functions in quiz_engine.c.
 *  - Show score history.
 *  - Use other modules for:
 *      - data (data_load / data_structs),
 *      - utilities (utils),
 *      - logging (logging).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "data_structs.h"
#include "utils.h"
#include "quiz_engine.h"
#include "logging.h"

/* Display score history from log file */
static void show_score_history(void) {
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
    
    /* Read test lines into a fixed-size array (simpler than malloc/free) */
    #define MAX_HISTORY_LINES 100
    char history[MAX_HISTORY_LINES][512];
    int line_count = 0;

    while (fgets(line, sizeof(line), fp) && line_count < MAX_HISTORY_LINES) {
        trim_newline(line);
        if (strstr(line, "[TEST]")) {
            strncpy(history[line_count], line, sizeof(history[line_count]) - 1);
            history[line_count][sizeof(history[line_count]) - 1] = '\0';
            line_count++;
        }
    }

    fclose(fp);

    /* Display results */
    printf("Test Results (showing last results):\n");
    printf("====================================\n\n");

    int max_display = 50;
    int start_idx;
    if (line_count > max_display) {
        start_idx = line_count - max_display;
    } else {
        start_idx = 0;
    }

    /* Display in reverse order (most recent first) */
    for (int i = line_count - 1; i >= start_idx; i--) {
        printf("%s\n", history[i]);
    }

    if (line_count > max_display) {
        printf("\n... (showing last %d results)\n", max_display);
    } else {
        printf("\nTotal test results shown: %d\n", line_count);
    }
    printf("\nPress Enter to return to menu...");
    char wait[8];
    read_line(wait, sizeof(wait));
}

/* Per-chapter submenu / dispatcher */
static void run_chapter_menu(const Chapter *chap, const char *username, int mode) {
    /* mode: 0 = Learn, 1 = Test */
    /* Build a simple log message (no fancy snprintf) */
    char log_msg[256];
    char mode_text[16];

    if (mode == 0) {
        strcpy(mode_text, "Learn");
    } else {
        strcpy(mode_text, "Test");
    }

    strcpy(log_msg, "User: ");
    strcat(log_msg, username);
    strcat(log_msg, " | Mode: ");
    strcat(log_msg, mode_text);
    strcat(log_msg, " | Chapter: ");
    strcat(log_msg, chap->name);

    log_event("INFO", log_msg);
    
    if (mode == 0) {
        run_learn_mode(chap);
    } else {
        run_test_mode(chap, username);
    }
}

/* Main menu - choose mode first, then chapter */
static void main_menu(const char *username) {
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
            if (mode == 0) {
                printf("Mode: Learn\n\n");
            } else {
                printf("Mode: Test\n\n");
            }
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
                run_chapter_menu(&CHAPTERS[chap_sel-1], username, mode);
            }
        }
    }
}

int main(void) {
    srand((unsigned)time(NULL));
    setup_console_utf8();
    clear_screen();
    
    log_event("INFO", "Program started");
    
    printf("=== Welcome to CalcQuiZ ===\n");
    printf("Please enter your username (leave blank = guest): ");
    char username[64];
    read_line(username, sizeof(username));
    if (username[0] == '\0') strcpy(username, "guest");
    
    /* Simple log message instead of using snprintf */
    char log_msg[128];
    strcpy(log_msg, "User logged in: ");
    strcat(log_msg, username);
    log_event("INFO", log_msg);
    
    main_menu(username);
    
    clear_screen();
    printf("Thank you for using. The program will now close.\n");
    log_event("INFO", "Program exited");
    return 0;
}

