#include <SDL2/SDL.h>
#include <iostream>
#include <memory>
#include <vector>
#include <cstdlib>
#include <ctime>

// Screen dimensions
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// Game Object Class
class GameObject {
public:
    int x, y, width, height;
    SDL_Color color;
    int xVel, yVel;

    GameObject(int x, int y, int width, int height, SDL_Color color)
        : x(x), y(y), width(width), height(height), color(color), xVel(0), yVel(0) {}

    virtual void update() {
        // Move object
        x += xVel;
        y += yVel;

        // Check screen boundaries
        if (x < 0 || x + width > SCREEN_WIDTH) {
            xVel = -xVel;
            x += xVel;
        }
        if (y < 0 || y + height > SCREEN_HEIGHT) {
            yVel = -yVel;
            y += yVel;
        }
    }

    void render(SDL_Renderer* renderer) {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_Rect rect = {x, y, width, height};
        SDL_RenderFillRect(renderer, &rect);
    }

    SDL_Rect getRect() const {
        return SDL_Rect{x, y, width, height};
    }
};

// Player Class (derived from GameObject)
class Player : public GameObject {
public:
    int health;

    Player(int x, int y, int width, int height, SDL_Color color, int health)
        : GameObject(x, y, width, height, color), health(health) {}

    void resetPosition() {
        x = SCREEN_WIDTH / 2 - width / 2;
        y = SCREEN_HEIGHT / 2 - height / 2;
    }

    void update() override {
        const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
        int speed = 5;
        if (currentKeyStates[SDL_SCANCODE_UP] && y > 0) {
            y -= speed;
        }
        if (currentKeyStates[SDL_SCANCODE_DOWN] && y < SCREEN_HEIGHT - height) {
            y += speed;
        }
        if (currentKeyStates[SDL_SCANCODE_LEFT] && x > 0) {
            x -= speed;
        }
        if (currentKeyStates[SDL_SCANCODE_RIGHT] && x < SCREEN_WIDTH - width) {
            x += speed;
        }
    }
};

// Game Engine Class
class GameEngine {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
    std::unique_ptr<Player> player;
    std::vector<std::unique_ptr<GameObject>> enemies;

public:
    GameEngine() : window(nullptr), renderer(nullptr), running(false), player(nullptr) {}

    bool init() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }

        window = SDL_CreateWindow("Simple 2D Game Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (!window) {
            std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }

        running = true;
        player = std::make_unique<Player>(SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT / 2 - 25, 50, 50, SDL_Color{255, 0, 0, 255}, 3);

        // Seed random number generator
        std::srand(std::time(0));

        // Create some enemies
        for (int i = 0; i < 5; ++i) {
            int x = std::rand() % (SCREEN_WIDTH - 50);
            int y = std::rand() % (SCREEN_HEIGHT - 50);
            int xVel = (std::rand() % 5 + 1) * (std::rand() % 2 ? 1 : -1);
            int yVel = (std::rand() % 5 + 1) * (std::rand() % 2 ? 1 : -1);
            enemies.push_back(std::make_unique<GameObject>(x, y, 50, 50, SDL_Color{0, 255, 0, 255}));
            enemies.back()->xVel = xVel;
            enemies.back()->yVel = yVel;
        }
        return true;
    }

    void handleEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
    }

    void update() {
        player->update();
        for (auto& enemy : enemies) {
            enemy->update();
        }
        checkCollisions();
    }

    void checkCollisions() {
        for (const auto& enemy : enemies) {
            if (checkCollision(player->getRect(), enemy->getRect())) {
                std::cout << "Collision detected! Health: " << --player->health << std::endl;
                player->resetPosition();
                if (player->health <= 0) {
                    std::cout << "Game Over!" << std::endl;
                    running = false;
                }
            }
        }
    }

    bool checkCollision(const SDL_Rect& a, const SDL_Rect& b) {
        return (a.x < b.x + b.w && a.x + a.w > b.x &&
                a.y < b.y + b.h && a.y + a.h > b.y);
    }

    void render() {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        player->render(renderer);
        for (const auto& enemy : enemies) {
            enemy->render(renderer);
        }

        SDL_RenderPresent(renderer);
    }

    void clean() {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    bool isRunning() {
        return running;
    }

    void run() {
        const int FPS = 60;
        const int frameDelay = 1000 / FPS;
        Uint32 frameStart;
        int frameTime;

        while (isRunning()) {
            frameStart = SDL_GetTicks();

            handleEvents();
            update();
            render();

            frameTime = SDL_GetTicks() - frameStart;

            if (frameDelay > frameTime) {
                SDL_Delay(frameDelay - frameTime);
            }
        }
    }
};

int main(int argc, char* args[]) {
    GameEngine game;

    if (!game.init()) {
        std::cerr << "Failed to initialize the game engine!" << std::endl;
        return -1;
    }

    game.run();
    game.clean();
    return 0;
}
