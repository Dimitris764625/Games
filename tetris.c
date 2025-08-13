#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <ncurses.h>
#include <stdbool.h>

// Add function prototype at the beginning
long millis();

#define WIDTH 10
#define HEIGHT 20
#define BLOCK_SIZE 4

// Tetrimino shapes
const int shapes[7][4][4][4] = {
    // I
    {
        {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
        {{0,0,1,0}, {0,0,1,0}, {0,0,1,0}, {0,0,1,0}},
        {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,1,0,0}}
    },
    // J
    {
        {{1,0,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,1,0}, {0,1,0,0}, {0,1,0,0}, {0,0,0,0}},
        {{0,0,0,0}, {1,1,1,0}, {0,0,1,0}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,0,0}, {1,1,0,0}, {0,0,0,0}}
    },
    // L
    {
        {{0,0,1,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,0,0}, {0,1,1,0}, {0,0,0,0}},
        {{0,0,0,0}, {1,1,1,0}, {1,0,0,0}, {0,0,0,0}},
        {{1,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,0,0,0}}
    },
    // O
    {
        {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}}
    },
    // S
    {
        {{0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,1,0}, {0,0,1,0}, {0,0,0,0}},
        {{0,0,0,0}, {0,1,1,0}, {1,1,0,0}, {0,0,0,0}},
        {{1,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,0,0,0}}
    },
    // T
    {
        {{0,1,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,1,0}, {0,1,0,0}, {0,0,0,0}},
        {{0,0,0,0}, {1,1,1,0}, {0,1,0,0}, {0,0,0,0}},
        {{0,1,0,0}, {1,1,0,0}, {0,1,0,0}, {0,0,0,0}}
    },
    // Z
    {
        {{1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,0,1,0}, {0,1,1,0}, {0,1,0,0}, {0,0,0,0}},
        {{0,0,0,0}, {1,1,0,0}, {0,1,1,0}, {0,0,0,0}},
        {{0,1,0,0}, {1,1,0,0}, {1,0,0,0}, {0,0,0,0}}
    }
};

// Colors for tetriminos
const int colors[7] = {
    COLOR_CYAN,    // I
    COLOR_BLUE,    // J
    COLOR_WHITE,   // L
    COLOR_YELLOW,  // O
    COLOR_GREEN,   // S
    COLOR_MAGENTA, // T
    COLOR_RED      // Z
};

int board[HEIGHT][WIDTH] = {0};
int score = 0;
int level = 1;
int lines_cleared = 0;
int fall_speed = 1000; // ms

typedef struct {
    int x;
    int y;
    int type;
    int rotation;
} Tetrimino;

Tetrimino current;

void init_game() {
    // Initialize board
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            board[y][x] = 0;
        }
    }
    
    score = 0;
    level = 1;
    lines_cleared = 0;
    fall_speed = 1000;
    
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    
    // Initialize colors
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_CYAN, COLOR_BLACK);
        init_pair(2, COLOR_BLUE, COLOR_BLACK);
        init_pair(3, COLOR_WHITE, COLOR_BLACK);
        init_pair(4, COLOR_YELLOW, COLOR_BLACK);
        init_pair(5, COLOR_GREEN, COLOR_BLACK);
        init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(7, COLOR_RED, COLOR_BLACK);
    }
}

void draw_board() {
    clear();
    
    // Draw border
    for (int y = 0; y < HEIGHT; y++) {
        mvprintw(y, 0, "|");
        mvprintw(y, WIDTH * 2 + 1, "|");
    }
    for (int x = 0; x < WIDTH * 2 + 2; x++) {
        mvprintw(HEIGHT, x, "-");
    }
    
    // Draw board
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (board[y][x]) {
                attron(COLOR_PAIR(board[y][x]));
                mvprintw(y, x * 2 + 1, "[]");
                attroff(COLOR_PAIR(board[y][x]));
            }
        }
    }
    
    // Draw current tetrimino
    for (int y = 0; y < BLOCK_SIZE; y++) {
        for (int x = 0; x < BLOCK_SIZE; x++) {
            if (shapes[current.type][current.rotation][y][x]) {
                attron(COLOR_PAIR(colors[current.type]));
                mvprintw(current.y + y, (current.x + x) * 2 + 1, "[]");
                attroff(COLOR_PAIR(colors[current.type]));
            }
        }
    }
    
    // Draw score and level
    mvprintw(1, WIDTH * 2 + 5, "Score: %d", score);
    mvprintw(3, WIDTH * 2 + 5, "Level: %d", level);
    mvprintw(5, WIDTH * 2 + 5, "Lines: %d", lines_cleared);
    
    // Draw controls
    mvprintw(8, WIDTH * 2 + 5, "Controls:");
    mvprintw(9, WIDTH * 2 + 5, "Left:  ←");
    mvprintw(10, WIDTH * 2 + 5, "Right: →");
    mvprintw(11, WIDTH * 2 + 5, "Rotate: ↑");
    mvprintw(12, WIDTH * 2 + 5, "Drop: ↓");
    mvprintw(13, WIDTH * 2 + 5, "Quit: q");
    
    refresh();
}

Tetrimino new_tetrimino() {
    Tetrimino t;
    t.type = rand() % 7;
    t.rotation = 0;
    t.x = WIDTH / 2 - BLOCK_SIZE / 2;
    t.y = 0;
    return t;
}

bool check_collision(Tetrimino t) {
    for (int y = 0; y < BLOCK_SIZE; y++) {
        for (int x = 0; x < BLOCK_SIZE; x++) {
            if (shapes[t.type][t.rotation][y][x]) {
                int board_x = t.x + x;
                int board_y = t.y + y;
                
                if (board_x < 0 || board_x >= WIDTH || board_y >= HEIGHT) {
                    return true;
                }
                
                if (board_y >= 0 && board[board_y][board_x]) {
                    return true;
                }
            }
        }
    }
    return false;
}

void merge_tetrimino() {
    for (int y = 0; y < BLOCK_SIZE; y++) {
        for (int x = 0; x < BLOCK_SIZE; x++) {
            if (shapes[current.type][current.rotation][y][x]) {
                int board_x = current.x + x;
                int board_y = current.y + y;
                if (board_y >= 0) {
                    board[board_y][board_x] = colors[current.type];
                }
            }
        }
    }
}

void clear_lines() {
    int lines_to_clear = 0;
    
    for (int y = HEIGHT - 1; y >= 0; y--) {
        bool line_complete = true;
        for (int x = 0; x < WIDTH; x++) {
            if (!board[y][x]) {
                line_complete = false;
                break;
            }
        }
        
        if (line_complete) {
            lines_to_clear++;
            // Move all lines above down
            for (int y2 = y; y2 > 0; y2--) {
                for (int x = 0; x < WIDTH; x++) {
                    board[y2][x] = board[y2 - 1][x];
                }
            }
            // Clear top line
            for (int x = 0; x < WIDTH; x++) {
                board[0][x] = 0;
            }
            y++; // Check the same line again
        }
    }
    
    if (lines_to_clear > 0) {
        // Update score
        switch (lines_to_clear) {
            case 1: score += 100 * level; break;
            case 2: score += 300 * level; break;
            case 3: score += 500 * level; break;
            case 4: score += 800 * level; break;
        }
        
        lines_cleared += lines_to_clear;
        
        // Update level every 10 lines
        level = lines_cleared / 10 + 1;
        
        // Increase speed
        fall_speed = 1000 - (level - 1) * 100;
        if (fall_speed < 100) fall_speed = 100;
    }
}

void rotate_tetrimino() {
    Tetrimino temp = current;
    temp.rotation = (temp.rotation + 1) % 4;
    
    // Try wall kicks
    if (!check_collision(temp)) {
        current = temp;
        return;
    }
    
    // Try moving left
    temp.x--;
    if (!check_collision(temp)) {
        current = temp;
        return;
    }
    
    // Try moving right
    temp.x += 2;
    if (!check_collision(temp)) {
        current = temp;
        return;
    }
    
    // Try moving left again (for I piece)
    temp.x -= 3;
    if (!check_collision(temp)) {
        current = temp;
        return;
    }
}

void game_loop() {
    srand(time(NULL));
    current = new_tetrimino();
    long last_fall = 0;
    bool game_over = false;
    
    while (!game_over) {
        long now = millis();
        
        // Handle input
        int ch = getch();
        switch (ch) {
            case KEY_LEFT:
                current.x--;
                if (check_collision(current)) current.x++;
                break;
            case KEY_RIGHT:
                current.x++;
                if (check_collision(current)) current.x--;
                break;
            case KEY_DOWN:
                current.y++;
                if (check_collision(current)) {
                    current.y--;
                    merge_tetrimino();
                    clear_lines();
                    current = new_tetrimino();
                    if (check_collision(current)) {
                        game_over = true;
                    }
                }
                last_fall = now;
                break;
            case KEY_UP:
                rotate_tetrimino();
                break;
            case 'q':
                game_over = true;
                break;
        }
        
        // Automatic falling
        if (now - last_fall > fall_speed) {
            current.y++;
            if (check_collision(current)) {
                current.y--;
                merge_tetrimino();
                clear_lines();
                current = new_tetrimino();
                if (check_collision(current)) {
                    game_over = true;
                }
            }
            last_fall = now;
        }
        
        draw_board();
        usleep(10000); // Small delay to prevent CPU overuse
    }
    
    // Game over screen
    clear();
    mvprintw(HEIGHT / 2, WIDTH - 5, "GAME OVER");
    mvprintw(HEIGHT / 2 + 1, WIDTH - 8, "Final Score: %d", score);
    mvprintw(HEIGHT / 2 + 3, WIDTH - 10, "Press any key to exit");
    refresh();
    nodelay(stdscr, FALSE);
    getch();
}

long millis() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

int main() {
    init_game();
    game_loop();
    endwin();
    return 0;
}
