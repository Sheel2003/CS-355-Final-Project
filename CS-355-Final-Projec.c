#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <curses.h>
#include <signal.h>

#define LENGTH 100

int snakeX[LENGTH], snakeY[LENGTH], snakeLength, fruitX, fruitY, score, MAXY, MAXX;
char direction;

void draw() {
    clear();
    printw("Welcome to the Snake Game!\n");
    printw("Score: %d\n", score);
    printw("Instructions:\n");
    printw("Use arrow keys to control the snake.\n");
    printw("Press 'Ctrl + C' to exit.\n");
    printw("+"); // Top-left corner of border
    for (int i = 0; i < MAXX; i++) printw("-");
    printw("+\n");
    for (int i = 0; i < MAXY; i++) {
        printw("|"); // Left border
        for (int j = 0; j < MAXX; j++) {
            if (i == snakeX[0] && j == snakeY[0])
                printw("0"); // Snake head
            else if (i == fruitX && j == fruitY)
                printw("*"); // Fruit
            else {
                int isSnakeBody = 0;
                for (int k = 1; k < snakeLength; k++) {
                    if (snakeX[k] == i && snakeY[k] == j) {
                        printw("o"); // Snake body
                        isSnakeBody = 1;
                        break;
                    }
                }
                if (!isSnakeBody)
                    printw(" ");
            }
        }
        printw("|\n"); // Right border
    }
    printw("+"); // Bottom-left corner of border
    for (int i = 0; i < MAXX; i++) printw("-");
    printw("+\n");
    refresh();
}

void input() {
    int ch = getch(); // Use int instead of char to handle arrow keys
    switch(ch) {
        case KEY_UP:
            if (direction != 1) direction = 0; // Change direction to UP if not already moving DOWN
            break;
        case KEY_DOWN:
            if (direction != 0) direction = 1; // Change direction to DOWN if not already moving UP
            break;
        case KEY_LEFT:
            if (direction != 3) direction = 2; // Change direction to LEFT if not already moving RIGHT
            break;
        case KEY_RIGHT:
            if (direction != 2) direction = 3; // Change direction to RIGHT if not already moving LEFT
            break;
        default:
            break;
    }
}

void moveSnake() {
    for (int i = snakeLength - 1; i > 0; i--) {
        snakeX[i] = snakeX[i - 1];
        snakeY[i] = snakeY[i - 1];
    }
    switch(direction) {
        case 0: // UP
            snakeX[0]--;
            break;
        case 1: // DOWN
            snakeX[0]++;
            break;
        case 2: // LEFT
            snakeY[0]--;
            break;
        case 3: // RIGHT
            snakeY[0]++;
            break;
        default:
            break;
    }
}

void checkCollision() {
    // Adjusted for borders
    if (snakeX[0] == -1 || snakeX[0] == MAXY || snakeY[0] == -1 || snakeY[0] == MAXX) {
        endwin();
        printf("Game Over! Final Score: %d\n", score);
        exit(0);
    }
    for (int i = 1; i < snakeLength; i++) {
        if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
            endwin();
            printf("Game Over! Final Score: %d\n", score);
            exit(0);
        }
    }
    if (snakeX[0] == fruitX && snakeY[0] == fruitY) {
        score++;
        snakeLength++;
        fruitX = rand() % (MAXY - 2) + 1;
        fruitY = rand() % (MAXX - 2) + 1;
    }
}

void cleanup(int signal) {
    endwin();
    printf("Exiting the game. Final Score: %d\n", score);
    exit(0);
}

int main() {
    initscr();
    clear();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(500); // Initial timeout value

    signal(SIGINT, cleanup); // Signal handling for Ctrl + C

    getmaxyx(stdscr, MAXY, MAXX); // Get terminal size
    MAXY -= 7; // Smallest numbers that work for me
    MAXX -= 3;

    snakeLength = 5; // Initial length set to five characters
    direction = rand() % 4; // Random initial direction: 0-UP, 1-DOWN, 2-LEFT, 3-RIGHT
    snakeX[0] = MAXY / 2;
    snakeY[0] = MAXX / 2;

    srand(time(NULL));
    fruitX = rand() % (MAXY - 2) + 1;
    fruitY = rand() % (MAXX - 2) + 1;
    score = 0;

    int perimeter = 2 * (MAXY - 1) + 2 * (MAXX - 1); // Perimeter of the border

    while(1) {
        draw();
        input();
        moveSnake();
        checkCollision();
        
        // Adjust snake speed based on its length
        timeout(200 - (snakeLength * 10 / (perimeter / 2)));
    }

    endwin();
    return 0;
}
