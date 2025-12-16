#ifndef LOGGING_H
#define LOGGING_H

/* Log test results to file */
void log_test_result(const char *username, const char *chap_code, int score, int total);

/* Log general events (like program start, mode selection, etc.) */
void log_event(const char *level, const char *message);

#endif /* LOGGING_H */

