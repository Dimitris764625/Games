#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

#define BALL_DELAY 50000
#define AI_DIFFICULTY 0.8 // Lower is harder (0.5-0.9 recommended)

typedef struct {
    int x, y;
    int original_x, original_y;
} Ball;

typedef struct {
    int y, original_y;
    int x;
    int size; // Now paddle size is part of the paddle struct
} Paddle;

typedef struct {
    int x, y;
    int type; // 1: enlarge paddle, 2: speed boost, 3: extra points
    bool active;
    time_t spawn_time;
} PowerUp;

// Game state
bool game_over = false;
int player_score = 0;
int opponent_score = 0;
int game_mode = 1; // 1: single player, 2: two players
int speed_boost = 0;
int ball_delay = BALL_DELAY;
const int INITIAL_PADDLE_SIZE = 4; // Initial paddle size

void init_ball(Ball *ball) {
    ball->original_x = COLS / 2;
    ball->original_y = LINES / 2;
    ball->x = ball->original_x;
    ball->y = ball->original_y;
}

void init_paddle(Paddle *paddle, int x_pos) {
    paddle->original_y = LINES / 2 - INITIAL_PADDLE_SIZE / 2;
    paddle->y = paddle->original_y;
    paddle->x = x_pos;
    paddle->size = INITIAL_PADDLE_SIZE;
}

void init_powerup(PowerUp *powerup) {
    powerup->active = false;
}

void draw_border() {
    for (int i = 0; i < COLS; i++) {
        mvprintw(0, i, "-");
        mvprintw(LINES - 1, i, "-");
    }
    for (int i = 0; i < LINES; i++) {
        mvprintw(i, 0, "|");
        mvprintw(i, COLS - 1, "|");
    }
}

void draw_ball(Ball *ball) {
    mvprintw(ball->y, ball->x, "O");
}

void draw_paddle(Paddle *paddle) {
    for (int i = 0; i < paddle->size; i++) {
        mvprintw(paddle->y + i, paddle->x, "|");
    }
}

void draw_powerup(PowerUp *powerup) {
    if (powerup->active) {
        attron(COLOR_PAIR(1));
        char symbol;
        switch (powerup->type) {
            case 1: symbol = 'E'; break; // Enlarge
            case 2: symbol = 'S'; break; // Speed
            case 3: symbol = 'P'; break; // Points
            default: symbol = '?';
        }
        mvprintw(powerup->y, powerup->x, "%c", symbol);
        attroff(COLOR_PAIR(1));
    }
}

void draw_scores() {
    mvprintw(1, COLS / 2 - 4, "%02d - %02d", player_score, opponent_score);
}

void draw_instructions() {
    mvprintw(LINES - 2, 2, "Q: Quit | P: Pause | R: Reset | M: Change Mode");
}

void reset_game(Ball *ball, Paddle *player, Paddle *opponent) {
    ball->x = ball->original_x;
    ball->y = ball->original_y;
    player->y = player->original_y;
    opponent->y = opponent->original_y;
    player->size = INITIAL_PADDLE_SIZE;
    opponent->size = INITIAL_PADDLE_SIZE;
    speed_boost = 0;
    ball_delay = BALL_DELAY;
}

void spawn_powerup(PowerUp *powerup, Ball *ball) {
    if (rand() % 100 < 15 && !powerup->active) { // 15% chance to spawn
        powerup->x = rand() % (COLS - 4) + 2;
        powerup->y = rand() % (LINES - 4) + 2;
        powerup->type = rand() % 3 + 1;
        powerup->active = true;
        powerup->spawn_time = time(NULL);
    }
}

void check_powerup_collision(PowerUp *powerup, Ball *ball, Paddle *player, Paddle *opponent) {
    if (powerup->active && 
        ((ball->x >= powerup->x - 1 && ball->x <= powerup->x + 1) &&
        (ball->y >= powerup->y - 1 && ball->y <= powerup->y + 1))) {
        
        // Apply powerup effect
        switch (powerup->type) {
            case 1: // Enlarge paddle
                if (ball->x < COLS / 2) {
                    // Player paddle
                    if (player->size < 8) player->size += 2;
                } else {
                    // Opponent paddle
                    if (opponent->size < 8) opponent->size += 2;
                }
                break;
            case 2: // Speed boost
                speed_boost = 100;
                ball_delay = BALL_DELAY / 2;
                break;
            case 3: // Extra points
                if (ball->x < COLS / 2) {
                    player_score += 2;
                } else {
                    opponent_score += 2;
                }
                break;
        }
        powerup->active = false;
    }
    
    // Powerup timeout (5 seconds)
    if (powerup->active && difftime(time(NULL), powerup->spawn_time) > 5) {
        powerup->active = false;
    }
}

void move_ball(Ball *ball, int *ball_dir_x, int *ball_dir_y, Paddle *player, Paddle *opponent) {
    // Move ball
    ball->x += *ball_dir_x;
    ball->y += *ball_dir_y;
    
    // Check collision with top and bottom
    if (ball->y <= 1 || ball->y >= LINES - 2) {
        *ball_dir_y *= -1;
    }
    
    // Check collision with paddles
    if ((ball->x == player->x + 1 && 
         ball->y >= player->y && 
         ball->y < player->y + player->size) ||
        (ball->x == opponent->x - 1 && 
         ball->y >= opponent->y && 
         ball->y < opponent->y + opponent->size)) {
        *ball_dir_x *= -1;
        
        // Add some randomness to the bounce
        if (rand() % 2) {
            *ball_dir_y = (*ball_dir_y == 1) ? -1 : 1;
        }
    }
    
    // Check if ball went past paddles
    if (ball->x <= 1) {
        opponent_score++;
        reset_game(ball, player, opponent);
        *ball_dir_x = 1;
    } else if (ball->x >= COLS - 2) {
        player_score++;
        reset_game(ball, player, opponent);
        *ball_dir_x = -1;
    }
}

void move_ai_paddle(Paddle *paddle, Ball *ball, int ball_dir_x) {
    // Only move if ball is coming towards AI
    if (ball_dir_x > 0) {
        // Move paddle towards ball with some imperfection
        if (paddle->y + paddle->size / 2 < ball->y && rand() / (float)RAND_MAX > AI_DIFFICULTY) {
            if (paddle->y < LINES - paddle->size - 2) {
                paddle->y++;
            }
        } else if (paddle->y + paddle->size / 2 > ball->y && rand() / (float)RAND_MAX > AI_DIFFICULTY) {
            if (paddle->y > 1) {
                paddle->y--;
            }
        }
    }
}

void handle_input(int ch, Paddle *player, Paddle *opponent, Ball *ball, int *ball_dir_x, int *ball_dir_y) {
    switch (ch) {
        case 'q':
        case 'Q':
            game_over = true;
            break;
        case 'w':
        case 'W':
            if (player->y > 1) {
                player->y--;
            }
            break;
        case 's':
        case 'S':
            if (player->y < LINES - player->size - 2) {
                player->y++;
            }
            break;
        case KEY_UP:
            if (game_mode == 2 && opponent->y > 1) {
                opponent->y--;
            }
            break;
        case KEY_DOWN:
            if (game_mode == 2 && opponent->y < LINES - opponent->size - 2) {
                opponent->y++;
            }
            break;
        case 'r':
        case 'R':
            player_score = 0;
            opponent_score = 0;
            reset_game(ball, player, opponent);
            *ball_dir_x = (rand() % 2) ? 1 : -1;
            *ball_dir_y = (rand() % 2) ? 1 : -1;
            break;
        case 'm':
        case 'M':
            game_mode = (game_mode == 1) ? 2 : 1;
            reset_game(ball, player, opponent);
            *ball_dir_x = (rand() % 2) ? 1 : -1;
            *ball_dir_y = (rand() % 2) ? 1 : -1;
            break;
        case 'p':
        case 'P':
            // Pause game
            while ((ch = getch()) != 'p' && ch != 'P') {
                mvprintw(LINES / 2, COLS / 2 - 5, "PAUSED");
                refresh();
                usleep(100000);
            }
            break;
    }
}

int main() {
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    nodelay(stdscr, TRUE);
    
    // Initialize colors
    start_color();
    init_pair(1, COLOR_YELLOW, COLOR_BLACK); // For powerups
    
    // Initialize game objects
    Ball ball;
    Paddle player, opponent;
    PowerUp powerup;
    
    init_ball(&ball);
    init_paddle(&player, 2);
    init_paddle(&opponent, COLS - 3);
    init_powerup(&powerup);
    
    // Ball direction (1 or -1 for x and y)
    int ball_dir_x = (rand() % 2) ? 1 : -1;
    int ball_dir_y = (rand() % 2) ? 1 : -1;
    
    // Seed random number generator
    srand(time(NULL));
    
    // Main game loop
    while (!game_over) {
        // Handle input
        int ch = getch();
        handle_input(ch, &player, &opponent, &ball, &ball_dir_x, &ball_dir_y);
        
        // AI movement in single player mode
        if (game_mode == 1) {
            move_ai_paddle(&opponent, &ball, ball_dir_x);
        }
        
        // Move ball
        move_ball(&ball, &ball_dir_x, &ball_dir_y, &player, &opponent);
        
        // Handle powerups
        spawn_powerup(&powerup, &ball);
        check_powerup_collision(&powerup, &ball, &player, &opponent);
        
        // Decrease speed boost over time
        if (speed_boost > 0) {
            speed_boost--;
            if (speed_boost == 0) {
                ball_delay = BALL_DELAY;
            }
        }
        
        // Draw everything
        clear();
        draw_border();
        draw_ball(&ball);
        draw_paddle(&player);
        draw_paddle(&opponent);
        draw_powerup(&powerup);
        draw_scores();
        draw_instructions();
        
        refresh();
        usleep(ball_delay);
    }
    
    // Clean up
    endwin();
    return 0;
}
