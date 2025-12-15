#include "data_load.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

/* Parse quiz file into Question array; return count loaded */
/* Format: question, 4 options, answer (1-4) */
int load_quiz_from_file(const char *path, Question *out, int max_out) {
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
int fallback_quiz(const Chapter *chap, Question *out, int max_out) {
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
void shuffle_indices(int *idx, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int t = idx[i];
        idx[i] = idx[j];
        idx[j] = t;
    }
}

