#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
/* Define Windows version requirements for newer console features */
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601 /* Windows 7+ */
#endif
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

#define MAX_QUESTIONS 64
#define MAX_TEXT 512
#define LOG_FILE "calculus.log"

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

static Chapter CHAPTERS[] = {
    {"LIM", "Limits", "quiz_limits.txt"},
    {"DER", "Derivatives", "quiz_derivative.txt"},
    {"APP", "Applications", ""}
};
static const int CHAPTER_COUNT = 3;

/* Utility: trim newline */
static void trim_newline(char *s) {
    if (!s) return;
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r')) {
        s[n-1] = '\0';
        n--;
    }
}

/* Utility: read a line from stdin safely */
static void read_line(char *buf, size_t sz) {
    if (!fgets(buf, (int)sz, stdin)) {
        buf[0] = '\0';
        clearerr(stdin);
    } else {
        trim_newline(buf);
    }
}

/* Utility: clear screen (Windows) */
static void clear_screen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/* Console UTF-8 setup */
static void setup_console_utf8(void) {
#ifdef _WIN32
    /* Set UTF-8 code pages */
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    
    /* Enable virtual terminal processing for better Unicode support (Windows 10+) */
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode); /* Ignore error if not supported */
        }
    }
    
    /* Try to set a font that supports mathematical symbols */
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
        /* Use CONSOLE_FONT_INFOEX if available (Windows Vista+) */
        typedef struct _CONSOLE_FONT_INFOEX_WRAP {
            ULONG cbSize;
            DWORD nFont;
            COORD dwFontSize;
            UINT FontFamily;
            UINT FontWeight;
            WCHAR FaceName[LF_FACESIZE];
        } CONSOLE_FONT_INFOEX_WRAP;
        
        typedef BOOL (WINAPI *GetCurrentConsoleFontExProc)(HANDLE, BOOL, void*);
        typedef BOOL (WINAPI *SetCurrentConsoleFontExProc)(HANDLE, BOOL, void*);
        
        HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
        if (hKernel32) {
            GetCurrentConsoleFontExProc pGetFont = (GetCurrentConsoleFontExProc)
                GetProcAddress(hKernel32, "GetCurrentConsoleFontEx");
            SetCurrentConsoleFontExProc pSetFont = (SetCurrentConsoleFontExProc)
                GetProcAddress(hKernel32, "SetCurrentConsoleFontEx");
            
            if (pGetFont && pSetFont) {
                CONSOLE_FONT_INFOEX_WRAP cfi;
                memset(&cfi, 0, sizeof(cfi));
                cfi.cbSize = sizeof(cfi);
                if (pGetFont(hConsole, FALSE, &cfi)) {
                    /* Try to use Consolas or a font that supports Unicode */
                    #ifdef __MINGW32__
                        wcscpy(cfi.FaceName, L"Consolas");
                    #else
                        wcscpy_s(cfi.FaceName, LF_FACESIZE, L"Consolas");
                    #endif
                    cfi.dwFontSize.X = 0;
                    cfi.dwFontSize.Y = 16;
                    cfi.FontFamily = FF_DONTCARE;
                    cfi.FontWeight = FW_NORMAL;
                    pSetFont(hConsole, FALSE, &cfi); /* Ignore error if fails */
                }
            }
        }
    }
    
    /* Set console buffer size for better display */
    HANDLE hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsoleOutput != INVALID_HANDLE_VALUE) {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(hConsoleOutput, &csbi)) {
            COORD newSize;
            newSize.X = csbi.dwSize.X;
            newSize.Y = 3000; /* Increase buffer for scrolling */
            SetConsoleScreenBufferSize(hConsoleOutput, newSize);
        }
    }
#endif
}

/* Forward declarations */
static void run_learn_mode(const Chapter *chap);
static void run_test_mode(const Chapter *chap, const char *username);

/* Lesson reading functions removed - program now focuses on quiz-based learning */

/* Parse quiz file into Question array; return count loaded */
/* Format: question, 4 options, answer (1-4), optional explanation */
static int load_quiz_from_file(const char *path, Question *out, int max_out) {
#ifdef _WIN32
    /* On Windows, try to open with UTF-8 encoding */
    FILE *fp = NULL;
    /* Try with explicit UTF-8 mode (Windows-specific) */
    #ifdef __MINGW32__
        /* MinGW: use regular fopen, encoding handled by console */
        fp = fopen(path, "r");
    #else
        /* MSVC: try UTF-8 mode */
        if (fopen_s(&fp, path, "r, ccs=UTF-8") != 0) {
            fp = fopen(path, "r");
        }
    #endif
#else
    FILE *fp = fopen(path, "r");
#endif
    if (!fp) return 0;
    char line[MAX_TEXT];
    int count = 0;
    while (count < max_out) {
        char q[MAX_TEXT] = {0};
        char opt[4][MAX_TEXT] = {{0}};
        char ans_line[MAX_TEXT] = {0};
        char expl[MAX_TEXT] = {0};

        /* Read question (skip blank lines) */
        while (fgets(line, sizeof(line), fp)) {
            trim_newline(line);
            if (line[0] == '\0') continue;
            strncpy(q, line, sizeof(q)-1);
            break;
        }
        if (q[0] == '\0') break; /* no more */

        /* Read 4 options */
        int ok = 1;
        for (int i = 0; i < 4; i++) {
            if (!fgets(line, sizeof(line), fp)) { ok = 0; break; }
            trim_newline(line);
            strncpy(opt[i], line, sizeof(opt[i])-1);
        }

        /* Read answer line */
        if (ok) {
            if (!fgets(ans_line, sizeof(ans_line), fp)) ok = 0;
            else trim_newline(ans_line);
        }
        
        /* Note: Quiz files don't contain explanations - they're only in fallback quizzes */
        /* If we wanted to support explanations in files, we'd need a marker like "EXPLANATION:" */
        
        if (!ok) break;

        int ans = atoi(ans_line); /* expects 1..4 */
        if (ans < 1 || ans > 4) ans = 1;

        strncpy(out[count].prompt, q, sizeof(out[count].prompt)-1);
        for (int i = 0; i < 4; i++) {
            strncpy(out[count].options[i], opt[i], sizeof(out[count].options[i])-1);
        }
        out[count].correctIndex = ans - 1;
        strncpy(out[count].explanation, expl, sizeof(out[count].explanation)-1);
        count++;
    }
    fclose(fp);
    return count;
}

/* Fallback quiz for each chapter; returns count */
static int fallback_quiz(const Chapter *chap, Question *out, int max_out) {
    int count = 0;
    if (strcmp(chap->code, "LIM") == 0) {
        /* 5 sample questions with explanations */
        Question q[] = {
            {"lim_{x->2} (x^2 - 4)/(x - 2) = ?",
             {"2", "4", "6", "Does not exist"}, 1,
             "Factor the numerator: x^2 - 4 = (x-2)(x+2). Cancel (x-2) to get lim_{x->2} (x+2) = 4."},
            {"If lim_{x->a} f(x) = L, this means?",
             {"f(a)=L definitely", "f(x) approaches L as x approaches a", "f(x)=L for all x", "None of the above"}, 1,
             "A limit describes the behavior of f(x) as x approaches a, not necessarily the value at a. The limit can exist even if f(a) is undefined."},
            {"For 0/0 cases, we usually use?",
             {"Direct substitution", "Factoring/multiplying by conjugate", "Change variable to t", "Conclude limit does not exist"}, 1,
             "When direct substitution gives 0/0, we use algebraic manipulation like factoring or multiplying by the conjugate to simplify the expression."},
            {"lim_{x->0} sin x / x = ?",
             {"0", "1", "Cannot be determined", "Depends on direction"}, 1,
             "This is a fundamental limit. Using L'Hospital's rule or geometric arguments, lim_{x->0} sin(x)/x = 1."},
            {"lim_{x->∞} 1/x = ?",
             {"0", "1", "∞", "Undefined"}, 0,
             "As x becomes very large, 1/x becomes very small and approaches 0. This is a basic limit at infinity."}
        };
        int n = (int)(sizeof(q)/sizeof(q[0]));
        for (int i = 0; i < n && count < max_out; i++) out[count++] = q[i];
    } else if (strcmp(chap->code, "DER") == 0) {
        Question q[] = {
            {"The definition of f'(x) is?",
             {"Average slope", "lim_{h->0} (f(x+h)-f(x))/h", "Second derivative", "Integral of f"}, 1,
             "The derivative is defined as the limit of the difference quotient: f'(x) = lim_{h->0} (f(x+h)-f(x))/h, which gives the instantaneous rate of change."},
            {"d/dx [x^3] = ?",
             {"3x^2", "x^2", "3x", "x^3"}, 0,
             "Using the power rule: d/dx [x^n] = n*x^(n-1). For x^3, we get 3*x^(3-1) = 3x^2."},
            {"(fg)' = ?",
             {"f'g'", "f'g + fg'", "f'g - fg'", "f'g / fg'"}, 1,
             "The product rule states that the derivative of a product is: (fg)' = f'g + fg'. This is different from the derivative of a sum."},
            {"d/dx [sin x] = ?",
             {"cos x", "-cos x", "sin x", "-sin x"}, 0,
             "The derivative of sin(x) is cos(x). This is a fundamental trigonometric derivative that should be memorized."},
            {"Chain rule is used when?",
             {"Constant function", "Composite function g(h(x))", "Second derivative", "Inverse function"}, 1,
             "The chain rule is used to differentiate composite functions. If y = g(h(x)), then dy/dx = g'(h(x)) * h'(x)."}
        };
        int n = (int)(sizeof(q)/sizeof(q[0]));
        for (int i = 0; i < n && count < max_out; i++) out[count++] = q[i];
    } else {
        Question q[] = {
            {"How is optimization done?",
             {"Guess", "Find derivative and critical points", "Always integrate", "Use limits only"}, 1,
             "Optimization involves finding the derivative, setting it to zero to find critical points, and testing these points to determine maxima or minima."},
            {"Instantaneous velocity is?",
             {"Displacement/time", "Derivative of position with respect to time", "Acceleration", "Constant"}, 1,
             "Instantaneous velocity is the derivative of the position function with respect to time: v(t) = ds/dt. It represents the velocity at a specific moment."},
            {"Maximum-minimum points are tested with?",
             {"Average value", "Second derivative/test endpoints", "Integral", "Limit"}, 1,
             "To find maximum and minimum values, we test critical points using the second derivative test or by evaluating the function at critical points and endpoints."}
        };
        int n = (int)(sizeof(q)/sizeof(q[0]));
        for (int i = 0; i < n && count < max_out; i++) out[count++] = q[i];
    }
    return count;
}

/* Shuffle indices */
static void shuffle_indices(int *idx, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int t = idx[i];
        idx[i] = idx[j];
        idx[j] = t;
    }
}

/* Log test results to file */
static void log_test_result(const char *username, const char *chap_code, int score, int total) {
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
static void log_event(const char *level, const char *message) {
    FILE *fp = fopen(LOG_FILE, "a");
    if (!fp) return;
    
    time_t now = time(NULL);
    struct tm *lt = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", lt);
    
    fprintf(fp, "[%s] [%s] %s\n", timestamp, level, message);
    fclose(fp);
}

/* Learn mode: Show immediate feedback with explanations (no score recording) */
static void run_learn_mode(const Chapter *chap) {
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

    int idx[MAX_QUESTIONS];
    for (int i = 0; i < total_loaded; i++) idx[i] = i;
    shuffle_indices(idx, total_loaded);
    
    for (int k = 0; k < total_loaded; k++) {
        int qi = idx[k];
        Question *q = &all[qi];
        clear_screen();
        printf("=== Learn Mode: %s ===\n", chap->name);
        printf("Question %d/%d\n\n", k+1, total_loaded);
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
    printf("You have completed all %d questions!\n", total_loaded);
    printf("\nPress Enter to return to menu...");
    char wait[8];
    read_line(wait, sizeof(wait));
}

/* Test mode: Collect all answers, then show results and record score */
static void run_test_mode(const Chapter *chap, const char *username) {
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

    int idx[MAX_QUESTIONS];
    for (int i = 0; i < total_loaded; i++) idx[i] = i;
    shuffle_indices(idx, total_loaded);
    
    int ask_total = total_loaded < 10 ? total_loaded : 10;
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
    printf("\nResults saved to %s\n", LOG_FILE);
    printf("\nPress Enter to return to menu...");
    char wait[8];
    read_line(wait, sizeof(wait));
}

/* Per-chapter submenu */
static int chapter_menu(const Chapter *chap, const char *username, int mode) {
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
        printf("Choose: ");
        char inp[16];
        read_line(inp, sizeof(inp));
        int mode_sel = atoi(inp);
        
        if (mode_sel == 3) {
            break; /* exit */
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

int main(void) {
    srand((unsigned)time(NULL));
    setup_console_utf8();
    clear_screen();
    
    log_event("INFO", "Program started");
    
    printf("=== Welcome to Calculus 1 Tutor ===\n");
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
