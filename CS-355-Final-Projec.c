#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <curses.h>
#include <signal.h>
#include <unistd.h>

#define LENGTH 1000

typedef struct {
    int x;
    int y;
    bool active;
} PowerUp;

int snakeX[LENGTH], snakeY[LENGTH], snakeLength, trophy, trophyX, trophyY, MAXY, MAXX, halfPerimeter, color;
char direction;
time_t trophyExpiration;
bool paused = false;
PowerUp powerUp;

int isPowerUpOnSnake() {
    for (int i = 0; i < snakeLength; i++) {
        if (powerUp.x == snakeX[i] && powerUp.y == snakeY[i]) {
            return 1; // PowerUp is on the snake
        }
    }
    return 0; // PowerUp is not on the snake
}

void generatePowerUp() {
    if (!powerUp.active && rand() % 100 == 0) {
        do {
            // Generate random position within screen bounds
            powerUp.x = (rand() % (MAXY - 7)) + 6;
            powerUp.y = (rand() % (MAXX - 3)) + 1;
        } while (isPowerUpOnSnake() || powerUp.x == trophyX || powerUp.y == trophyY);

        powerUp.active = true;
        mvprintw(powerUp.x, powerUp.y, "+");
    }
}

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
    } while (abs(trophyX - snakeX[0]) > maxDistance || abs(trophyY - snakeY[0]) > maxDistance || isTrophyOnSnake() || (trophyX == powerUp.x && trophyY == powerUp.y));

    time_t currentTime;
    time(&currentTime);
    trophyExpiration = currentTime + (rand() % 9) + 1;
    return newTrophy;
}

void drawStatic() {
    printw("Welcome to the Snake Game!\n");
    printw("Score: %d\n", snakeLength);
    printw("Winning score: %d\n", halfPerimeter);
    printw("Press 'P' to pause the game.\n");
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

    // Draw the new head of the snake
    mvprintw(snakeX[0], snakeY[0], "0");

    // Draw the new body of the snake
    for (int i = 1; i < snakeLength; i++) {
        mvprintw(snakeX[i], snakeY[i], "o");
    }

    // Draw the score
    mvprintw(1, 0, "Score: %d", snakeLength);

    // Draw the new trophy
    mvprintw(trophyX, trophyY, "%d", trophy);

    // Draw the powerUp if active
    if (powerUp.active) {
        color++;
        attron(COLOR_PAIR(color % 8));
        mvprintw(powerUp.x, powerUp.y, "+");
        attroff(COLOR_PAIR(color % 8));
    }

    // Refresh the screen to apply changes
    refresh();
}

void gameOver(int signal) {
    clear(); // Clear the screen
    mvprintw(MAXY / 2, (MAXX - 18) / 2, "You lost! Final Score: %d", snakeLength);
    refresh(); // Refresh the screen to show the message
    napms(5000); // Delay for 5 sec
    endwin(); // End curses mode
    exit(0);
}



void input() {
    int ch = getch(); // Use int instead of char to handle arrow keys
    switch(ch) {
        case KEY_UP:
            if (direction != 1) direction = 0; // Change direction to UP if not already moving DOWN
            else
                gameOver(0);
            break;
        case KEY_DOWN:
            if (direction != 0) direction = 1; // Change direction to DOWN if not already moving UP
            else
                gameOver(0);
            break;
        case KEY_LEFT:
            if (direction != 3) direction = 2; // Change direction to LEFT if not already moving RIGHT
            else
                gameOver(0);
            break;
        case KEY_RIGHT:
            if (direction != 2) direction = 3; // Change direction to RIGHT if not already moving LEFT
            else
                gameOver(0);
            break;
        case 'p':
            paused = !paused; // Toggle pause state
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

void invincibleDrawDynamic() {

    // Draw the new trophy
    mvprintw(trophyX, trophyY, "%d", trophy);

    // Draw the new head of the snake
    attron(COLOR_PAIR(color % 8));
    mvprintw(snakeX[0], snakeY[0], "0");
    attroff(COLOR_PAIR(color % 8));

    // Draw the new body of the snake
    for (int i = 1; i < snakeLength; i++) {
        color++;
        if (snakeLength % 8 != 0) {
            attron(COLOR_PAIR(color % 8));
            mvprintw(snakeX[i], snakeY[i], "o");
            attroff(COLOR_PAIR(color % 8));
        } else {
            attron(COLOR_PAIR(color % 7));
            mvprintw(snakeX[i], snakeY[i], "o");
            attroff(COLOR_PAIR(color % 7));
        }
    }

    // Draw the score
    mvprintw(1, 0, "Score: %d", snakeLength);

    // Refresh the screen to apply changes
    refresh();
}

void invincibleCheckCollision() {
    // Adjusted for borders
    if (snakeX[0] == 5 || snakeX[0] == MAXY - 1 || snakeY[0] == 0 || snakeY[0] == MAXX - 2) {
        gameOver(0);
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

void invincibleInput() {
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
        case 'p':
            paused = !paused; // Toggle pause state
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
    // Checks if snake collides with powerup
    if (snakeX[0] == powerUp.x && snakeY[0] == powerUp.y) {
        time_t powerUpExpiration;
        time(&powerUpExpiration);
        powerUpExpiration += 20; // Power-up lasts for 20 seconds
        
        while(1) {
        if (!paused) {
            invincibleDrawDynamic(); // Draw dynamic elements (snake and trophies)
            invincibleInput();
            moveSnake();
            generatePowerUp();
            invincibleCheckCollision();
            refresh();

            int speed = halfPerimeter - snakeLength;
            if (speed > 20)
                usleep(speed * 2000); // Convert speed to microseconds
            else
                usleep(20 * 1000); // Convert 20 milliseconds to microseconds

            if (snakeLength >= halfPerimeter){
                endwin();
                printf("You Win! Max Length Reached On Your Screen: %d\n", halfPerimeter);
                exit(0);
            }
            time_t current;
            time(&current);
            if (current >= powerUpExpiration) {
                powerUp.active = false;
                break; // Exit the loop if power-up expires
            }
        }
        else {
            // Display a message indicating that the game is paused
            int middleX = (MAXX - 12 - 5) / 2;
            // Colors not working right //
            //start_color();
            //init_pair(1, COLOR_RED, COLOR_BLACK);
            //attron(COLOR_PAIR(1));
            mvprintw(5, (MAXX - 12 - 5) / 2, "Game Paused");
            //attroff(COLOR_PAIR(1));
            refresh();
            while (1) {
                if (getch() == 'p') {
                    paused = false;
                    //clear(); // Clear the screen before redrawing
                    //drawStatic(); // Redraw static elements
                    mvprintw(5, (MAXX - 12 - 5) / 2, "-----------");
                    break;
                }

            }
        }
    }
    }
}

int main() {
    initscr();
    clear();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(1); // Initial timeout value

    signal(SIGINT, gameOver); // Signal handling for Ctrl + C

    getmaxyx(stdscr, MAXY, MAXX); // Get terminal size

    srand(time(NULL));
    for (int i = 0; i < LENGTH; i++) {
        snakeX[i] = -1;
        snakeY[i] = -1;
    }
    snakeLength = 5; // Initial length set to five characters
    direction = rand() % 4; // Random initial direction: 0-UP, 1-DOWN, 2-LEFT, 3-RIGHT
    snakeX[0] = MAXY / 2;
    snakeY[0] = MAXX / 2;

    trophy = generateTrophy();

    // Shows score needed to win
    halfPerimeter = (MAXY - 1) + (MAXX - 1); // Perimeter of the border
    printw("To win the game, you'll need to reach a score of: %d", halfPerimeter);
    refresh();
    sleep(1);
    clear();

    drawStatic(); // Draw static elements at the start of the game

    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_CYAN, COLOR_BLACK);
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(7, COLOR_WHITE, COLOR_BLACK);

    while(1) {
        if (!paused) {
            drawDynamic(); // Draw dynamic elements (snake and trophies)
            input();
            moveSnake();
            generatePowerUp();
            checkCollision();
            refresh();

            int speed = halfPerimeter - snakeLength;
            if (speed > 20)
                usleep(speed * 1000); // Convert speed to microseconds
            else
                usleep(20 * 1000); // Convert 20 milliseconds to microseconds

            if (snakeLength >= halfPerimeter){
                endwin();
                printf("You Win! Max Length Reached On Your Screen: %d\n", halfPerimeter);
                exit(0);
            }
        }
        else {
            // Display a message indicating that the game is paused
            int middleX = (MAXX - 12 - 5) / 2;
            // Colors not working right //
            //start_color();
            //init_pair(1, COLOR_RED, COLOR_BLACK);
            //attron(COLOR_PAIR(1));
            mvprintw(5, (MAXX - 12 - 5) / 2, "Game Paused");
            //attroff(COLOR_PAIR(1));
            refresh();
            while (1) {
                if (getch() == 'p') {
                    paused = false;
                    //clear(); // Clear the screen before redrawing
                    //drawStatic(); // Redraw static elements
                    mvprintw(5, (MAXX - 12 - 5) / 2, "-----------");
                    break;
                }

            }
        }
    }
}
