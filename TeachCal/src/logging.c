#include "logging.h"
#include <stdio.h>
#include <time.h>
#include <string.h>

#define LOG_FILE "logs/quiz_score.log"

/* Log test results to file */
void log_test_result(const char *username, const char *chap_code, int score, int total) {
    FILE *fp = fopen(LOG_FILE, "a");
    if (!fp) return;
    
    time_t now = time(NULL);
    struct tm *lt = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", lt);
    
    double percentage = total > 0 ? (double)score / total * 100.0 : 0.0;
    
    /* Log format: [TIMESTAMP] [LEVEL] User: username | Chapter: code | Score: X/Y (Z%) */
    fprintf(fp, "[%s] [TEST] User: %s | Chapter: %s | Score: %d/%d (%.1f%%)\n", 
            timestamp, username, chap_code, score, total, percentage);
    
    fclose(fp);
}

/* Log general events (like program start, mode selection, etc.) */
void log_event(const char *level, const char *message) {
    FILE *fp = fopen(LOG_FILE, "a");
    if (!fp) return;
    
    time_t now = time(NULL);
    struct tm *lt = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", lt);
    
    fprintf(fp, "[%s] [%s] %s\n", timestamp, level, message);
    fclose(fp);
}

