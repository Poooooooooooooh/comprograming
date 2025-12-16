#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "data_structs.h"
#include "utils.h"
#include "quiz_engine.h"
#include "logging.h"

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
    
    char log_msg[128];
    snprintf(log_msg, sizeof(log_msg), "User logged in: %s", username);
    log_event("INFO", log_msg);
    
    main_menu(username);
    
    clear_screen();
    printf("Thank you for using. The program will now close.\n");
    log_event("INFO", "Program exited");
    return 0;
}

