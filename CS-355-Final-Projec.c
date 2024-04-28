#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <curses.h>
#include <signal.h>
#include <unistd.h>

#define LENGTH 1000

int snakeX[LENGTH], snakeY[LENGTH], snakeLength, trophy, trophyX, trophyY, MAXY, MAXX;
char direction;
time_t trophyExpiration;

int isTrophyOnSnake() {
    for (int i = 0; i < snakeLength; i++) {
        if (trophyX == snakeX[i] && trophyY == snakeY[i]) {
            return 1; // Trophy is on the snake
        }
    }
    return 0; // Trophy is not on the snake
}

int generateTrophy() {
    int newTrophy = (rand() % 9) + 1;
    int maxDistance = 15; // Maximum distance from the snake

    // Generate trophy position within the specified range
    do {
        trophyX = (rand() % (MAXY - 7)) + 6; // Generate random X position within screen bounds
        trophyY = (rand() % (MAXX - 3)) + 1; // Generate random Y position within screen bounds
    } while (abs(trophyX - snakeX[0]) > maxDistance || abs(trophyY - snakeY[0]) > maxDistance || isTrophyOnSnake());

    time_t currentTime;
    time(&currentTime);
    trophyExpiration = currentTime + (rand() % 9) + 1;
    return newTrophy;
}

void drawStatic() {
    printw("Welcome to the Snake Game!\n");
    printw("Score: %d\n", snakeLength);
    printw("Instructions:\n");
    printw("Use arrow keys to control the snake.\n");
    printw("Press 'Ctrl + C' to exit.\n");
    printw("+"); // Top-left corner of border
    for (int i = 3; i < MAXX; i++)
        printw("-");
    printw("+\n");
    for (int i = 7; i < MAXY; i++) {
        printw("|"); // Left border
        for (int j = 3; j < MAXX; j++) {
            printw(" ");
        }
        printw("|\n"); // Right border
    }
    printw("+"); // Bottom-left corner of border
    for (int i = 3; i < MAXX; i++)
        printw("-");
    printw("+\n");
}

void drawDynamic() {

    // Draw the new trophy
    mvprintw(trophyX, trophyY, "%d", trophy);

    // Draw the new head of the snake
    mvprintw(snakeX[0], snakeY[0], "0");

    // Draw the new body of the snake
    for (int i = 1; i < snakeLength; i++) {
        mvprintw(snakeX[i], snakeY[i], "o");
    }

    // Draw the score
    mvprintw(1, 0, "Score: %d", snakeLength);

    // Refresh the screen to apply changes
    refresh();
}

void gameOver(int signal) {
    endwin();
    printf("Game Over! Final Score: %d\n", snakeLength);
    exit(0);
}

void input() {
    int ch = getch(); // Use int instead of char to handle arrow keys
    switch(ch) {
        case KEY_UP:
            if (direction != 1) direction = 0; // Change direction to UP if not already moving DOWN
            else {
                gameOver(0);
            }
            break;
        case KEY_DOWN:
            if (direction != 0) direction = 1; // Change direction to DOWN if not already moving UP
            else {
                gameOver(0);
            }
            break;
        case KEY_LEFT:
            if (direction != 3) direction = 2; // Change direction to LEFT if not already moving RIGHT
            else {
                gameOver(0);
            }
            break;
        case KEY_RIGHT:
            if (direction != 2) direction = 3; // Change direction to RIGHT if not already moving LEFT
            else {
                gameOver(0);
            }
            break;
        default:
            break;
    }
}

void moveSnake() {
    mvprintw(snakeX[snakeLength - 1], snakeY[snakeLength - 1], " ");
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
    if (snakeX[0] == 5 || snakeX[0] == MAXY - 1 || snakeY[0] == 0 || snakeY[0] == MAXX - 2) {
        gameOver(0);
    }
    // Checks if the snake runs into itself
    for (int i = 1; i < snakeLength; i++) {
        if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
            gameOver(0);
        }
    }
    // Checks if the snake collides with trophy
    if (snakeX[0] == trophyX && snakeY[0] == trophyY) {
        snakeLength += trophy;
        trophy = generateTrophy();
    }
    // Checks if the trophy has expired
    time_t currentTime;
    time(&currentTime);
    if (currentTime >= trophyExpiration) {
        mvprintw(trophyX, trophyY, " ");
        trophy = generateTrophy();
    }
}

int main() {
    initscr();
    clear();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(500); // Initial timeout value

    signal(SIGINT, gameOver); // Signal handling for Ctrl + C

    getmaxyx(stdscr, MAXY, MAXX); // Get terminal size
    MAXY -= 0;
    MAXX -= 0;

    snakeLength = 5; // Initial length set to five characters
    direction = rand() % 4; // Random initial direction: 0-UP, 1-DOWN, 2-LEFT, 3-RIGHT
    snakeX[0] = MAXY / 2;
    snakeY[0] = MAXX / 2;

    srand(time(NULL));
    trophy = generateTrophy();

    // Shows score needed to win
    int halfPerimeter = (MAXY - 1) + (MAXX - 1); // Perimeter of the border
    printw("Winning score: %d", halfPerimeter);
    refresh();
    sleep(1);
    clear();

    drawStatic(); // Draw static elements at the start of the game

    while(1) {
        drawDynamic(); // Draw dynamic elements (snake and trophies)
        input();
        moveSnake();
        checkCollision();
        refresh();
        
        // Adjust snake speed based on its length
        int speed = halfPerimeter - snakeLength;
        if (speed > 20)
            timeout(speed);
        else
            timeout(20);

        if (snakeLength >= halfPerimeter){
            endwin();
            printf("You Win! Max Length Reached On Your Screen: %d\n", snakeLength);
            exit(0);
        }
    }
}
