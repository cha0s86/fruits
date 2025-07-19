// This is a simple agar.io-like game using SDL2
// Players move around, eat fruits to grow, and can shoot fruit projectiles
// Compile with: g++ fruits.cpp -o fruits -lSDL2
#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>

// === Window Constants ===
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

// === Structs ===
struct Projectile {
    float x, y;
    float vx, vy;
    int w, h;
    SDL_Color color;
    int owner; // 0 = player1, 1 = player2
};

// === Rendering Functions ===
void renderRect(SDL_Renderer* renderer, const SDL_Rect& rect, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &rect);
}

void renderPlayer(SDL_Renderer* renderer, const SDL_Rect& player, Uint8 alpha = 255) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_Color color = {255, 0, 0, alpha}; // Red
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &player);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, alpha);
    SDL_RenderDrawRect(renderer, &player);
}

void renderPlayer2(SDL_Renderer* renderer, const SDL_Rect& player, Uint8 alpha = 255) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_Color color = {0, 255, 0, alpha}; // Green
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &player);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, alpha);
    SDL_RenderDrawRect(renderer, &player);
}

void renderProjectile(SDL_Renderer* renderer, const Projectile& proj) {
    SDL_Rect rect = {static_cast<int>(proj.x), static_cast<int>(proj.y), proj.w, proj.h};
    renderRect(renderer, rect, proj.color);
}

// === Input Handling ===
void handlePlayerInput(SDL_Rect& player) {
    const Uint8* state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_W])    player.y -= 5;
    if (state[SDL_SCANCODE_S])    player.y += 5;
    if (state[SDL_SCANCODE_A])    player.x -= 5;
    if (state[SDL_SCANCODE_D])    player.x += 5;
}

void handlePlayer2Input(SDL_Rect& player2) {
    const Uint8* state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_UP])    player2.y -= 5;
    if (state[SDL_SCANCODE_DOWN])  player2.y += 5;
    if (state[SDL_SCANCODE_LEFT])  player2.x -= 5;
    if (state[SDL_SCANCODE_RIGHT]) player2.x += 5;
}

void handlePlayer2AI(SDL_Rect& player, const SDL_Rect& fruit) {
    if (player.x < fruit.x)      player.x += 5;
    else if (player.x > fruit.x) player.x -= 5;
    if (player.y < fruit.y)      player.y += 5;
    else if (player.y > fruit.y) player.y -= 5;
}

// === Utility Functions ===
void keepInBounds(SDL_Rect& rect) {
    if (rect.x < 0) rect.x = 0;
    if (rect.y < 0) rect.y = 0;
    if (rect.x + rect.w > WINDOW_WIDTH)  rect.x = WINDOW_WIDTH - rect.w;
    if (rect.y + rect.h > WINDOW_HEIGHT) rect.y = WINDOW_HEIGHT - rect.h;
}

void checkEatFruit(SDL_Rect& player, SDL_Rect& fruit) {
    if (SDL_HasIntersection(&player, &fruit)) {
        fruit.x = rand() % (WINDOW_WIDTH - 50);
        fruit.y = rand() % (WINDOW_HEIGHT - 50);
        player.w += 10;
        player.h += 10;
        keepInBounds(player);
    }
}

// === Main Game ===
int main(int argc, char* argv[]) {
    // SDL Initialization
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    SDL_Window* window = SDL_CreateWindow(
        "Fruits! - Agar.io-like Game",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN
    );
    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool useAI = false;
    if (argc > 1) {
        if (std::string(argv[1]) == "--ai") {
            useAI = true;
        } else {
            std::cout << "Wrong syntax: available flags are (--ai)\n";
            return 1;
        }
    }


    // Random seed
    srand(static_cast<unsigned int>(time(0)));

    // Players and fruits
    SDL_Rect player  = {rand() % (WINDOW_WIDTH - 50), rand() % (WINDOW_HEIGHT - 50), 50, 50};
    SDL_Rect player2 = {rand() % (WINDOW_WIDTH - 50), rand() % (WINDOW_HEIGHT - 50), 50, 50};
    SDL_Rect apple   = {rand() % (WINDOW_WIDTH - 50), rand() % (WINDOW_HEIGHT - 50), 50, 50};
    SDL_Rect pear    = {rand() % (WINDOW_WIDTH - 50), rand() % (WINDOW_HEIGHT - 50), 50, 50};
    SDL_Color appleColor = {255, 0, 0, 255};
    SDL_Color pearColor  = {0, 255, 0, 255};

    int player1Projectiles = 3;
    int player2Projectiles = 3;
    std::vector<Projectile> projectiles;
    bool running = true;
    bool mousePressed = false;
    SDL_Event event;

    // === Main Loop ===
    while (running) {
        // --- Event Handling ---
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
                player.x = WINDOW_WIDTH / 2 - 25;
                player.y = WINDOW_HEIGHT / 2 - 25;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) mousePressed = true;
            if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) mousePressed = false;
        }

        // --- Game Logic ---
        keepInBounds(player);
        keepInBounds(player2);
        // Check if player eats apple
        int prevW = player.w, prevH = player.h;
        checkEatFruit(player, apple);
        if (player.w > prevW || player.h > prevH) {
            player1Projectiles = 3;
        }
        // Check if player2 eats pear (AI)
        int prevW2 = player2.w, prevH2 = player2.h;
        checkEatFruit(player2, pear);
        if (player2.w > prevW2 || player2.h > prevH2) {
            player2Projectiles = 3;
        }
        // Shooting: add projectile on mouse click (player1)
        if (mousePressed && player1Projectiles > 0) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);
            float px = player.x + player.w / 2.0f - 10;
            float py = player.y + player.h / 2.0f - 10;
            float dx = mouseX - (player.x + player.w / 2.0f);
            float dy = mouseY - (player.y + player.h / 2.0f);
            float len = std::sqrt(dx * dx + dy * dy);
            if (len > 0) { dx /= len; dy /= len; }
            float speed = 10.0f;
            projectiles.push_back({px, py, dx * speed, dy * speed, 20, 20, appleColor, 0});
            player1Projectiles--;
            mousePressed = false; // Only shoot once per click
        }
        // AI shooting mechanism (only if useAI)
        if (useAI && player2Projectiles > 0) {
            bool aiCanShoot = true;
            for (const auto& proj : projectiles) {
                SDL_Rect projRect = {static_cast<int>(proj.x), static_cast<int>(proj.y), proj.w, proj.h};
                if (SDL_HasIntersection(&projRect, &player2)) {
                    aiCanShoot = false;
                    break;
                }
            }
            if (aiCanShoot) {
                float px = player2.x + player2.w / 2.0f - 10;
                float py = player2.y + player2.h / 2.0f - 10;
                float dx = (player.x + player.w / 2.0f) - (player2.x + player2.w / 2.0f);
                float dy = (player.y + player.h / 2.0f) - (player2.y + player2.h / 2.0f);
                float len = std::sqrt(dx * dx + dy * dy);
                if (len > 0) { dx /= len; dy /= len; }
                float speed = 10.0f;
                projectiles.push_back({px, py, dx * speed, dy * speed, 20, 20, pearColor, 1});
                player2Projectiles--;
            }
        }
        // Update projectiles
        for (auto& proj : projectiles) {
            proj.x += proj.vx;
            proj.y += proj.vy;
        }
        // Check projectile collision with player2 and player1, and remove on hit
        projectiles.erase(
            std::remove_if(projectiles.begin(), projectiles.end(), [&](const Projectile& proj) {
                SDL_Rect projRect = {static_cast<int>(proj.x), static_cast<int>(proj.y), proj.w, proj.h};
                // Only player1's projectiles can hit player2
                if (proj.owner == 0 && SDL_HasIntersection(&projRect, &player2)) {
                    player2.w = std::max(20, player2.w - 10); // Ensure player2 doesn't shrink below 20x20 and reduce size by 20
                    player2.h = std::max(20, player2.h - 10);
                    keepInBounds(player2);
                    return true;
                }
                // Only player2's projectiles can hit player1
                if (proj.owner == 1 && SDL_HasIntersection(&projRect, &player)) {
                    player.w = std::max(20, player.w - 10);
                    player.h = std::max(20, player.h - 10);
                    keepInBounds(player);
                    return true;
                }
                // Remove if out of bounds
                return proj.x < 0 || proj.y < 0 || proj.x + proj.w > WINDOW_WIDTH || proj.y + proj.h > WINDOW_HEIGHT;
            }),
            projectiles.end()
        );

        handlePlayerInput(player);

        // Check if program was run with AI flag
        if (useAI) {
            handlePlayer2AI(player2, pear);
        } else {
            handlePlayer2Input(player2);
        }

        // --- Rendering ---
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        renderRect(renderer, apple, appleColor);
        renderRect(renderer, pear, pearColor);
        // Determine transparency for overlap
        Uint8 alpha1 = 255, alpha2 = 255;
        if (SDL_HasIntersection(&player, &apple) || SDL_HasIntersection(&player, &pear) || SDL_HasIntersection(&player, &player2)) alpha1 = 128;
        if (SDL_HasIntersection(&player2, &apple) || SDL_HasIntersection(&player2, &pear) || SDL_HasIntersection(&player2, &player)) alpha2 = 128;
        if (player.w < player2.w) {
            renderPlayer(renderer, player, alpha1);
            renderPlayer2(renderer, player2, alpha2);
        } else {
            renderPlayer2(renderer, player2, alpha2);
            renderPlayer(renderer, player, alpha1);
        }
        for (const auto& proj : projectiles) renderProjectile(renderer, proj);
        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    // --- Cleanup ---
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
