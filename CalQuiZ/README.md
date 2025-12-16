# Calculus 1 Tutor

A modular C program for learning and testing calculus concepts through interactive quizzes. This application provides two modes: Learn Mode for practice with immediate feedback, and Test Mode for timed quizzes with score tracking.

## Project Structure

```
TeachCal/
├── data/
│   ├── quiz_limits.txt      # Limits quiz questions
│   └── quiz_derivative.txt  # Derivatives quiz questions
├── src/
│   ├── main.c               # Main entry point
│   ├── data_structs.h/c     # Data structures (Question, Chapter)
│   ├── data_load.h/c        # Quiz file loading and parsing
│   ├── quiz_engine.h/c      # Quiz logic and user interface
│   ├── logging.h/c          # File logging functionality
│   └── utils.h/c           # Utility functions (I/O, console setup)
├── CalQuiZ.exe              # Compiled executable (Windows)
└── README.md                # This file
```

## Features

- **Learn Mode**: Practice with immediate feedback and explanations (no score recording)
- **Test Mode**: Take timed quizzes and see your score with detailed results
- **Score History**: View your test score history (option 5 in main menu)
- **Customizable Questions**: Choose how many questions you want to answer
- **Multiple Chapters**: Limits, Derivatives, and Applications
- **UTF-8 Support**: Proper display of mathematical notation
- **Automatic Logging**: All quiz results are logged to `logs/quiz_score.log`

## Building

### Requirements
- GCC compiler (MinGW on Windows, gcc on Linux)
- C11 standard support

### Windows
```bash
gcc -Wall -Wextra -std=c11 src/main.c src/data_structs.c src/utils.c src/data_load.c src/logging.c src/quiz_engine.c -o CalQuiZ.exe
```

### Linux/Mac
```bash
gcc -Wall -Wextra -std=c11 src/main.c src/data_structs.c src/utils.c src/data_load.c src/logging.c src/quiz_engine.c -o CalQuiZ
```

The executable will be created in the root directory.

## Usage

1. Run the executable: `CalQuiZ.exe` (Windows) or `./CalQuiZ` (Linux/Mac)
2. Enter your username (or press Enter for "guest")
3. Choose a mode:
   - **1) Learn Mode**: Practice with immediate feedback
   - **2) Test Mode**: Take a test and see your score
   - **3) Exit program**
   - **5) Score History**: View your test score history
4. Select a chapter (Limits, Derivatives, or Applications)
5. Choose how many questions you want (or press Enter for all questions)
6. Answer the questions and see your results!

## Module Descriptions

### 1. Main Module (`main.c`)
- Program entry point
- Initializes random seed, console setup
- Handles username input
- Delegates to quiz engine

### 2. Data Structures (`data_structs.h/c`)
- Defines core data types: `Question` and `Chapter`
- Contains chapter definitions (Limits, Derivatives, Applications)
- Provides constants: `MAX_QUESTIONS`, `MAX_TEXT`

### 3. Data Loading (`data_load.h/c`)
- Loads quiz questions from text files
- Parses quiz file format
- Provides fallback questions if files are missing
- Shuffles questions for randomization

### 4. Quiz Engine (`quiz_engine.h/c`)
- Implements Learn Mode and Test Mode
- Handles user interface and menu navigation
- Manages question flow and answer collection
- Displays score history

### 5. Logging (`logging.h/c`)
- Logs test results to `logs/quiz_score.log`
- Logs general program events
- Automatically creates logs directory if needed

### 6. Utilities (`utils.h/c`)
- Cross-platform screen clearing
- Safe input reading
- UTF-8 console setup for Windows
- String processing utilities

## Quiz File Format

Quiz files in the `data/` directory follow this format:
```
Question text
Option 1
Option 2
Option 3
Option 4
Answer (1-4)
```

Example:
```
What is the derivative of x^3?
3x^2
x^2
3x
x^3
1
```

## Logging

Test results are automatically saved to `logs/quiz_score.log` with the following format:
```
[YYYY-MM-DD HH:MM:SS] [TEST] User: username | Chapter: code | Score: X/Y (Z%)
```

The logs directory is automatically created if it doesn't exist.

## Module Dependencies

```
main.c
  ├── data_structs.h
  ├── utils.h
  ├── quiz_engine.h
  └── logging.h

quiz_engine.c
  ├── data_structs.h
  ├── data_load.h
  ├── utils.h
  └── logging.h

data_load.c
  ├── data_structs.h
  └── utils.h

logging.c
  └── (standard library only)

utils.c
  └── (standard library + Windows API)
```

## License

This project is part of a computer programming course.

## Author

Poooooooooooooh

## Repository

https://github.com/Poooooooooooooh/comprograming.git

