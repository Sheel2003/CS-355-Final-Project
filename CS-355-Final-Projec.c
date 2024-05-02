#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <curses.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>

#define LENGTH 1000

void gameloop();

typedef struct {
    int x[LENGTH];
    int y[LENGTH];
    int length;
    char direction;
    bool invincible;
} Snake;

typedef struct {
    int x;
    int y;
    int value;
    time_t expiration;
} Trophy;

typedef struct {
    int x;
    int y;
    bool onScreen;
    time_t expiration;
} PowerUp;

int MAXY, MAXX, halfPerimeter, color;
char direction;
bool paused = false;
Snake snake;
Trophy trophy;
PowerUp powerUp;

int isItemOnSnake(int x, int y) {
    for (int i = 0; i < snake.length; i++) {
        if (x == snake.x[i] && y == snake.y[i]) {
            return 1; // Item is on the snake
        }
    }
    return 0; // Item is not on the snake
}

void generatePowerUp() {
    if (!powerUp.onScreen && !snake.invincible && rand() % 100 == 0) {
        do {
            // Generate random position within screen bounds
            powerUp.x = (rand() % (MAXY - 7)) + 6;
            powerUp.y = (rand() % (MAXX - 3)) + 1;
        } while (isItemOnSnake(powerUp.x, powerUp.y) || powerUp.x == trophy.x || powerUp.y == trophy.y);

        powerUp.onScreen = true;
    }
}

void generateTrophy() {
    trophy.value = (rand() % 9) + 1;
    int maxDistance = 15; // Maximum distance from the snake

    // Generate trophy position within the specified range
    do {
        trophy.x = (rand() % (MAXY - 7)) + 6; // Generate random X position within screen bounds
        trophy.y = (rand() % (MAXX - 3)) + 1; // Generate random Y position within screen bounds
    } while (abs(trophy.x - snake.x[0]) > maxDistance || abs(trophy.y - snake.y[0]) > maxDistance || isItemOnSnake(trophy.x, trophy.y) || (trophy.x == powerUp.x && trophy.y == powerUp.y));

    time_t currentTime;
    time(&currentTime);
    trophy.expiration = currentTime + (rand() % 9) + 1;
}

void drawStatic() {
    printw("Welcome to the Snake Game!\n");
    printw("Score: %d\n", snake.length);
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
    mvprintw(snake.x[0], snake.y[0], "0");

    if (!snake.invincible) {
        // Draw the new body of the snake
        for (int i = 1; i < snake.length; i++) {
            mvprintw(snake.x[i], snake.y[i], "o");
        }
    } else {
        // Draw the new head of the snake
        attron(COLOR_PAIR(color % 8));
        mvprintw(snake.x[0], snake.y[0], "0");
        attroff(COLOR_PAIR(color % 8));

        // Draw the new body of the snake
        for (int i = 1; i < snake.length; i++) {
            color++;
            if (snake.length % 8 != 0) {
                attron(COLOR_PAIR(color % 8));
                mvprintw(snake.x[i], snake.y[i], "o");
                attroff(COLOR_PAIR(color % 8));
            } else {
                attron(COLOR_PAIR(color % 7));
                mvprintw(snake.x[i], snake.y[i], "o");
                attroff(COLOR_PAIR(color % 7));
            }
        }
    }

    // Draw the score
    mvprintw(1, 0, "Score: %d", snake.length);

    // Draw the new trophy
    mvprintw(trophy.x, trophy.y, "%d", trophy.value);

    // Draw the powerUp if onScreen
    if (powerUp.onScreen) {
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
    mvprintw(MAXY / 2, (MAXX - 18) / 2, "You lost! Final Score: %d", snake.length);
    refresh(); // Refresh the screen to show the message
    usleep(5000000); // Delay for 5 sec
    endwin(); // End curses mode
    exit(0);
}

void input() {
    bool invincible = snake.invincible;
    int ch = getch(); // Use int instead of char to handle arrow keys
    switch(ch) {
        case KEY_UP:
            if (direction != 1) direction = 0; // Change direction to UP if not already moving DOWN
            else if (!invincible) gameOver(0);
            break;
        case KEY_DOWN:
            if (direction != 0) direction = 1; // Change direction to DOWN if not already moving UP
            else if (!invincible) gameOver(0);
            break;
        case KEY_LEFT:
            if (direction != 3) direction = 2; // Change direction to LEFT if not already moving RIGHT
            else if (!invincible) gameOver(0);
            break;
        case KEY_RIGHT:
            if (direction != 2) direction = 3; // Change direction to RIGHT if not already moving LEFT
            else if (!invincible) gameOver(0);
            break;
        case 'p':
        case 'P':
            paused = !paused; // Toggle pause state
            break;
        default:
            break;
    }
}

void moveSnake() {
    mvprintw(snake.x[snake.length - 1], snake.y[snake.length - 1], " ");
    for (int i = snake.length - 1; i > 0; i--) {
        snake.x[i] = snake.x[i - 1];
        snake.y[i] = snake.y[i - 1];
    }
    switch(direction) {
        case 0: // UP
            snake.x[0]--;
            break;
        case 1: // DOWN
            snake.x[0]++;
            break;
        case 2: // LEFT
            snake.y[0]--;
            break;
        case 3: // RIGHT
            snake.y[0]++;
            break;
        default:
            break;
    }
}

void checkCollision() {
    // Adjusted for borders
    if (snake.x[0] == 5 || snake.x[0] == MAXY - 1 || snake.y[0] == 0 || snake.y[0] == MAXX - 2) {
        gameOver(0);
    }
    if (!snake.invincible) {
        // Checks if the snake runs into itself
        for (int i = 1; i < snake.length; i++) {
            if (snake.x[0] == snake.x[i] && snake.y[0] == snake.y[i]) {
                gameOver(0);
            }
        }
    }
    // Checks if the snake collides with trophy
    if (snake.x[0] == trophy.x && snake.y[0] == trophy.y) {
        snake.length += trophy.value;
        generateTrophy();
    }
    // Checks if the trophy has expired
    time_t currentTime;
    time(&currentTime);
    if (currentTime >= trophy.expiration) {
        mvprintw(trophy.x, trophy.y, " ");
        generateTrophy();
    }
    // Checks if snake collides with powerup
    if (snake.x[0] == powerUp.x && snake.y[0] == powerUp.y) {
        snake.invincible= true;
        powerUp.onScreen = false;
        time(&powerUp.expiration);
        powerUp.expiration += 20; // Power-up lasts for 20 seconds
        
        while(1) {
        if (!paused) {
            time_t current;
            time(&current);
            if (current >= powerUp.expiration) {
                snake.invincible = false;
                break; // Exit the loop if power-up expires
            }
        }
        gameloop();
        }
    }
}

void gameloop() {
    if (!paused) {
        drawDynamic(); // Draw dynamic elements (snake and trophies)
        input();
        moveSnake();
        generatePowerUp();
        checkCollision();
        refresh();

        int speed = halfPerimeter - snake.length - 10;
        if (speed > 20)
            usleep(speed * 1000); // Convert speed to microseconds
        else
            usleep(20 * 1000); // Convert 20 milliseconds to microseconds

        if (snake.length >= halfPerimeter){
            endwin();
            printf("You Win! Max Length Reached On Your Screen: %d\n", halfPerimeter);
            exit(0);
        }
    } else {
        // Display a message indicating that the game is paused
        int middleX = (MAXX - 12 - 5) / 2;
        attron(COLOR_PAIR(1));
        mvprintw(5, middleX, "Game Paused");
        attroff(COLOR_PAIR(1));
        refresh();
        while (1) {
            int inputChar = getch();
            if (tolower(inputChar) == 'p') {
                paused = false;
                mvprintw(5, middleX, "-----------");
                break;
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
        snake.x[i] = -1;
        snake.y[i] = -1;
    }
    snake.length = 5; // Initial length set to five characters
    direction = rand() % 4; // Random initial direction: 0-UP, 1-DOWN, 2-LEFT, 3-RIGHT
    snake.x[0] = MAXY / 2;
    snake.y[0] = MAXX / 2;

    //generateTrophy();

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
        gameloop();
    }
}