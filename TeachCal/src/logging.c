#include "logging.h"
#include <stdio.h>
#include <time.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#define mkdir(path, mode) _mkdir(path)
#define access(path, mode) _access(path, mode)
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#define LOG_FILE "logs/quiz_score.log"
#define LOG_DIR "logs"

/* Ensure logs directory exists */
static void ensure_log_dir(void) {
#ifdef _WIN32
    if (_access(LOG_DIR, 0) != 0) {
        _mkdir(LOG_DIR);
    }
#else
    struct stat st = {0};
    if (stat(LOG_DIR, &st) == -1) {
        mkdir(LOG_DIR, 0700);
    }
#endif
}

/* Log test results to file */
void log_test_result(const char *username, const char *chap_code, int score, int total) {
    ensure_log_dir();
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
    ensure_log_dir();
    FILE *fp = fopen(LOG_FILE, "a");
    if (!fp) return;
    
    time_t now = time(NULL);
    struct tm *lt = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", lt);
    
    fprintf(fp, "[%s] [%s] %s\n", timestamp, level, message);
    fclose(fp);
}

