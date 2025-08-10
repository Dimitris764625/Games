#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <ncurses.h>

#define WIDTH 30
#define HEIGHT 20
#define INITIAL_LENGTH 3

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    Point *body;
    int length;
    int direction; // 0: up, 1: right, 2: down, 3: left
} Snake;

typedef struct {
    Point position;
    int is_special;
    time_t spawn_time;
} Food;

int game_over = 0;
int score = 0;
int high_score = 0;
int special_active = 0;
int wrap_around = 0; // Set to 1 to enable wrap-around mode

void init_snake(Snake *snake) {
    snake->body = malloc(INITIAL_LENGTH * sizeof(Point));
    snake->length = INITIAL_LENGTH;
    snake->direction = 1; // Start moving right
    
    // Initialize snake body in the middle of the screen
    for (int i = 0; i < INITIAL_LENGTH; i++) {
        snake->body[i].x = WIDTH / 2 - i;
        snake->body[i].y = HEIGHT / 2;
    }
}

void init_food(Food *food) {
    food->position.x = rand() % WIDTH;
    food->position.y = rand() % HEIGHT;
    food->is_special = 0;
    food->spawn_time = time(NULL);
}

void generate_food(Food *food, Snake *snake) {
    int valid;
    do {
        valid = 1;
        food->position.x = rand() % WIDTH;
        food->position.y = rand() % HEIGHT;
        
        // Check if food spawns on snake
        for (int i = 0; i < snake->length; i++) {
            if (food->position.x == snake->body[i].x && 
                food->position.y == snake->body[i].y) {
                valid = 0;
                break;
            }
        }
        
        // 20% chance for special food if none is currently active
        if (!special_active && (rand() % 5 == 0)) {
            food->is_special = 1;
            special_active = 1;
        } else {
            food->is_special = 0;
        }
        
    } while (!valid);
    
    food->spawn_time = time(NULL);
}

void draw_borders() {
    // Draw top and bottom borders
    for (int i = 0; i < WIDTH + 2; i++) {
        mvprintw(0, i, "#");
        mvprintw(HEIGHT + 1, i, "#");
    }
    
    // Draw left and right borders
    for (int i = 1; i < HEIGHT + 1; i++) {
        mvprintw(i, 0, "#");
        mvprintw(i, WIDTH + 1, "#");
    }
}

void draw_snake(Snake *snake) {
    for (int i = 0; i < snake->length; i++) {
        if (i == 0) {
            // Draw head differently
            attron(COLOR_PAIR(2));
            mvprintw(snake->body[i].y + 1, snake->body[i].x + 1, "O");
            attroff(COLOR_PAIR(2));
        } else {
            attron(COLOR_PAIR(1));
            mvprintw(snake->body[i].y + 1, snake->body[i].x + 1, "o");
            attroff(COLOR_PAIR(1));
        }
    }
}

void draw_food(Food *food) {
    if (food->is_special) {
        attron(COLOR_PAIR(3));
        mvprintw(food->position.y + 1, food->position.x + 1, "@");
        attroff(COLOR_PAIR(3));
    } else {
        attron(COLOR_PAIR(4));
        mvprintw(food->position.y + 1, food->position.x + 1, "*");
        attroff(COLOR_PAIR(4));
    }
}

void update_snake(Snake *snake) {
    // Move each segment to the position of the previous segment
    for (int i = snake->length - 1; i > 0; i--) {
        snake->body[i] = snake->body[i - 1];
    }
    
    // Move head based on direction
    switch (snake->direction) {
        case 0: // Up
            snake->body[0].y--;
            break;
        case 1: // Right
            snake->body[0].x++;
            break;
        case 2: // Down
            snake->body[0].y++;
            break;
        case 3: // Left
            snake->body[0].x--;
            break;
    }
    
    // Handle wrap-around if enabled
    if (wrap_around) {
        if (snake->body[0].x >= WIDTH) snake->body[0].x = 0;
        if (snake->body[0].x < 0) snake->body[0].x = WIDTH - 1;
        if (snake->body[0].y >= HEIGHT) snake->body[0].y = 0;
        if (snake->body[0].y < 0) snake->body[0].y = HEIGHT - 1;
    }
}

int check_collision(Snake *snake) {
    // Check wall collision (if wrap-around is disabled)
    if (!wrap_around) {
        if (snake->body[0].x < 0 || snake->body[0].x >= WIDTH ||
            snake->body[0].y < 0 || snake->body[0].y >= HEIGHT) {
            return 1;
        }
    }
    
    // Check self collision
    for (int i = 1; i < snake->length; i++) {
        if (snake->body[0].x == snake->body[i].x && 
            snake->body[0].y == snake->body[i].y) {
            return 1;
        }
    }
    
    return 0;
}

void check_food(Snake *snake, Food *food) {
    if (snake->body[0].x == food->position.x && 
        snake->body[0].y == food->position.y) {
        
        // Increase score based on food type
        if (food->is_special) {
            score += 5;
            special_active = 0;
        } else {
            score += 1;
        }
        
        // Grow snake
        snake->length++;
        snake->body = realloc(snake->body, snake->length * sizeof(Point));
        snake->body[snake->length - 1] = snake->body[snake->length - 2];
        
        // Generate new food
        generate_food(food, snake);
        
        // Update high score
        if (score > high_score) {
            high_score = score;
        }
    }
    
    // Check if special food has expired (10 seconds)
    if (food->is_special && difftime(time(NULL), food->spawn_time) > 10) {
        special_active = 0;
        generate_food(food, snake);
    }
}

void game_loop() {
    Snake snake;
    Food food;
    
    init_snake(&snake);
    init_food(&food);
    
    int ch;
    int speed = 100000; // Initial speed (microseconds)
    time_t last_move = time(NULL);
    
    while (!game_over) {
        // Handle input
        timeout(0); // Non-blocking input
        ch = getch();
        
        switch (ch) {
            case KEY_UP:
                if (snake.direction != 2) snake.direction = 0;
                break;
            case KEY_RIGHT:
                if (snake.direction != 3) snake.direction = 1;
                break;
            case KEY_DOWN:
                if (snake.direction != 0) snake.direction = 2;
                break;
            case KEY_LEFT:
                if (snake.direction != 1) snake.direction = 3;
                break;
            case 'w':
                if (snake.direction != 2) snake.direction = 0;
                break;
            case 'd':
                if (snake.direction != 3) snake.direction = 1;
                break;
            case 's':
                if (snake.direction != 0) snake.direction = 2;
                break;
            case 'a':
                if (snake.direction != 1) snake.direction = 3;
                break;
            case 'q':
                game_over = 1;
                break;
            case 'p':
                // Pause game
                while (getch() != 'p') {
                    mvprintw(HEIGHT / 2, WIDTH / 2 - 5, "PAUSED");
                    refresh();
                    usleep(100000);
                }
                break;
        }
        
        // Game logic
        if (difftime(time(NULL), last_move) >= 0.1) { // Move every 0.1 seconds
            update_snake(&snake);
            if (check_collision(&snake)) {
                game_over = 1;
            }
            check_food(&snake, &food);
            last_move = time(NULL);
            
            // Increase speed as score increases
            speed = 100000 - (score * 500);
            if (speed < 50000) speed = 50000; // Minimum speed
        }
        
        // Drawing
        clear();
        draw_borders();
        draw_snake(&snake);
        draw_food(&food);
        
        // Display score
        mvprintw(HEIGHT + 2, 0, "Score: %d", score);
        mvprintw(HEIGHT + 3, 0, "High Score: %d", high_score);
        if (special_active) {
            int time_left = 10 - difftime(time(NULL), food.spawn_time);
            mvprintw(HEIGHT + 4, 0, "Special Food: %d seconds left", time_left);
        }
        
        refresh();
        usleep(speed);
    }
    
    free(snake.body);
}

void show_game_over() {
    clear();
    mvprintw(HEIGHT / 2 - 1, WIDTH / 2 - 5, "GAME OVER");
    mvprintw(HEIGHT / 2, WIDTH / 2 - 8, "Final Score: %d", score);
    mvprintw(HEIGHT / 2 + 1, WIDTH / 2 - 10, "High Score: %d", high_score);
    mvprintw(HEIGHT / 2 + 3, WIDTH / 2 - 12, "Press any key to exit");
    refresh();
    getch();
}

int main() {
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    // Initialize colors
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK); // Snake body
    init_pair(2, COLOR_YELLOW, COLOR_BLACK); // Snake head
    init_pair(3, COLOR_MAGENTA, COLOR_BLACK); // Special food
    init_pair(4, COLOR_RED, COLOR_BLACK); // Regular food
    
    // Seed random number generator
    srand(time(NULL));
    
    // Main game loop
    do {
        game_over = 0;
        score = 0;
        game_loop();
        show_game_over();
        
        // Ask to play again
        clear();
        mvprintw(HEIGHT / 2 - 1, WIDTH / 2 - 10, "Play again? (y/n)");
        refresh();
        
        int ch;
        while ((ch = getch()) != 'y' && ch != 'n' && ch != 'Y' && ch != 'N');
        if (ch == 'n' || ch == 'N') break;
        
        // Toggle wrap-around mode if 'm' is pressed during game over
        if (ch == 'm' || ch == 'M') {
            wrap_around = !wrap_around;
        }
    } while (1);
    
    // Clean up ncurses
    endwin();
    
    return 0;
}
