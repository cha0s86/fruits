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
#include <limits>

// === Window Constants ===
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

// === Structs ===
struct Projectile {
    float x, y;
    float vx, vy;
    int w, h;
    SDL_Color color;
    int owner; // 0 = player1, 1 = player2, 2 = lemonAI, 3 = blueberryAI, 4 = orangeAI
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
void handleAppleMovement(SDL_Rect& player) {
    const Uint8* state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_W])    player.y -= 5;
    if (state[SDL_SCANCODE_S])    player.y += 5;
    if (state[SDL_SCANCODE_A])    player.x -= 5;
    if (state[SDL_SCANCODE_D])    player.x += 5;
}

void handlePearMovement(SDL_Rect& player2) {
    const Uint8* state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_UP])    player2.y -= 5;
    if (state[SDL_SCANCODE_DOWN])  player2.y += 5;
    if (state[SDL_SCANCODE_LEFT])  player2.x -= 5;
    if (state[SDL_SCANCODE_RIGHT]) player2.x += 5;
}

void handleAIMovement(SDL_Rect& player, const SDL_Rect& fruit) {
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

void printControls() {
    std::cout << "\n=== FRUITS GAME CONTROLS ===\n";
    std::cout << "Apple (Red): WASD to move, Mouse to shoot (AI mode only)\n";
    std::cout << "Pear (Green): Arrow keys to move\n";
    std::cout << "T: Toggle AI shooting (AI mode only)\n";
    std::cout << "P: Show projectile counts\n";
    std::cout << "ESC: Quit game\n";
    std::cout << "=============================\n\n";
}

void printProjectileCounts(int player1Projectiles, int player2Projectiles, bool useAI) {
    std::cout << "Projectile Counts:\n";
    std::cout << "Apple (Red): " << player1Projectiles << " projectiles left\n";
    if (useAI) {
        std::cout << "AI (Green): " << player2Projectiles << " projectiles left\n";
    } else {
        std::cout << "Pear (Green): " << player2Projectiles << " projectiles left\n";
    }
    std::cout << "------------------------\n";
}

void checkWinCondition(SDL_Rect* fruits, int numFruits, const std::string* fruitNames, bool& running) {
    for (int i = 0; i < numFruits; ++i) {
        // Check if player1 wins (covers whole screen)
        if (fruits[i].w >= WINDOW_WIDTH && fruits[i].h >= WINDOW_HEIGHT) {
            std::cout << "\n" << fruitNames[i] << " won!\n";
            std::cout << fruitNames[i] << " covered the entire screen!\n";
            running = false;
            return;
        }
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

    // Parse command line arguments
    bool useAI = false;
    bool aiCanShoot = false; // Default to AI shooting disabled
    int numAIs = 1; // Default to 1 AI
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--ai") {
            useAI = true;
            if (argc > 2) {
                try {
                    numAIs = std::stoi(argv[2]);
                    if (numAIs < 1) numAIs = 1;
                    if (numAIs > 4) numAIs = 4;
                } catch (...) {
                    numAIs = 1;
                }
            }
            std::cout << "Starting in PvAI mode (Apple vs " << numAIs << " AI opponent(s))\n";
        } else {
            std::cout << "Usage: " << argv[0] << " [--ai N]\n";
            std::cout << "  --ai N: PvAI mode (Apple vs N AIs, N=1-4)\n";
            std::cout << "  No arguments: PvP mode (Apple vs Pear)\n";
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 1;
        }
    } else {
        std::cout << "Starting in PvP mode (Apple vs Pear)\n";
    }

    // Print controls
    printControls();

    // Random seed
    srand(static_cast<unsigned int>(time(0)));

    // Add colors
    const SDL_Color appleColor = {255, 0, 0, 255};          // Apple
    const SDL_Color pearColor = { 0, 255, 0, 255};          // Pear
    const SDL_Color lemonColor = {255, 255, 0, 255};        // Yellow
    const SDL_Color blueberryColor = {0, 128, 255, 255};    // Blue
    const SDL_Color orangeColor = {255, 140, 0, 255};       // Orange

    // Fruits and their colors
    SDL_Rect apple  = {rand() % (WINDOW_WIDTH - 50), rand() % (WINDOW_HEIGHT - 50), 50, 50}; // Apple (was player1)
    SDL_Rect pear   = {rand() % (WINDOW_WIDTH - 50), rand() % (WINDOW_HEIGHT - 50), 50, 50}; // Pear (was player2/AI 0)
    SDL_Rect lemon  = {rand() % (WINDOW_WIDTH - 50), rand() % (WINDOW_HEIGHT - 50), 50, 50}; // Lemon (AI 1)
    SDL_Rect blueberry = {rand() % (WINDOW_WIDTH - 50), rand() % (WINDOW_HEIGHT - 50), 50, 50}; // Blueberry (AI 2)
    SDL_Rect orange = {rand() % (WINDOW_WIDTH - 50), rand() % (WINDOW_HEIGHT - 50), 50, 50}; // Orange (AI 3)
    SDL_Rect fruitRects[5] = {apple, pear, lemon, blueberry, orange};
    SDL_Color fruitColors[5] = {
        {255, 0, 0, 255},      // Apple (Red)
        {0, 255, 0, 255},      // Pear (Green)
        {255, 255, 0, 255},    // Lemon (Yellow)
        {0, 128, 255, 255},    // Blueberry (Blue)
        {255, 140, 0, 255}     // Orange
    };
    int fruitProjectiles[5] = {3, 3, 3, 3, 3};
    SDL_Rect fruitTargets[5];
    for (int i = 0; i < 5; ++i) {
        fruitTargets[i] = {rand() % (WINDOW_WIDTH - 20), rand() % (WINDOW_HEIGHT - 20), 20, 20};
    }
    std::vector<Projectile> projectiles;
    bool running = true;
    bool appleShootPressed = false;
    SDL_Event event;

    // Add fruit names for debug and win messages
    std::string fruitNames[5] = {"Apple", "Pear", "Lemon", "Blueberry", "Orange"};

    // === Main Loop ===
    while (running) {
        // --- Event Handling ---
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        running = false;
                        break;
                    case SDLK_t:
                        aiCanShoot = !aiCanShoot;
                        std::cout << "AI shooting: " << (aiCanShoot ? "ENABLED" : "DISABLED") << "\n";
                        break;
                    case SDLK_p:
                        printProjectileCounts(fruitProjectiles[0], fruitProjectiles[1], useAI);
                        break;
                }
            }
            // Mouse events for Apple (only in AI mode)
            if (useAI && event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                appleShootPressed = true;
            }
            if (useAI && event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
                appleShootPressed = false;
            }
        }

        // --- Game Logic ---
        for (int i = 0; i < numAIs + 1; ++i) keepInBounds(fruitRects[i]);
        // Remove old hardcoded win check for Apple and Pear
        // Check win condition for all fruits
        checkWinCondition(fruitRects, numAIs + 1, fruitNames, running);
        if (!running) break;
        // Check if each fruit eats its target
        for (int i = 0; i < numAIs + 1; ++i) {
            int prevW = fruitRects[i].w, prevH = fruitRects[i].h;
            checkEatFruit(fruitRects[i], fruitTargets[i]);
            if (fruitRects[i].w > prevW || fruitRects[i].h > prevH) {
                fruitProjectiles[i] = 3;
                std::cout << fruitNames[i] << " ate its fruit! Projectiles reset to 3\n";
            }
        }
        // Apple shooting (mouse click - only in AI mode)
        if (appleShootPressed && fruitProjectiles[0] > 0 && useAI) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);
            float px = fruitRects[0].x + fruitRects[0].w / 2.0f - 10;
            float py = fruitRects[0].y + fruitRects[0].h / 2.0f - 10;
            float dx = mouseX - (fruitRects[0].x + fruitRects[0].w / 2.0f);
            float dy = mouseY - (fruitRects[0].y + fruitRects[0].h / 2.0f);
            float len = std::sqrt(dx * dx + dy * dy);
            if (len > 0) { dx /= len; dy /= len; }
            float speed = 10.0f;
            projectiles.push_back({px, py, dx * speed, dy * speed, 20, 20, fruitColors[0], 0});
            fruitProjectiles[0]--;
            appleShootPressed = false;
        }
        // AI shooting mechanism (only if useAI and aiCanShoot is true)
        if (useAI && aiCanShoot) {
            for (int i = 1; i < numAIs + 1; ++i) {
                if (fruitProjectiles[i] > 0) {
                    // Find nearest target (any other fruit, not self)
                    float minDist = std::numeric_limits<float>::max();
                    int targetIdx = 0;
                    for (int j = 0; j < numAIs + 1; ++j) {
                        if (i == j) continue;
                        float dx = (fruitRects[j].x + fruitRects[j].w/2.0f) - (fruitRects[i].x + fruitRects[i].w/2.0f);
                        float dy = (fruitRects[j].y + fruitRects[j].h/2.0f) - (fruitRects[i].y + fruitRects[i].h/2.0f);
                        float dist = dx*dx + dy*dy;
                        if (dist < minDist) {
                            minDist = dist;
                            targetIdx = j;
                        }
                    }
                    // Only shoot if not intersecting with self
                    bool canShootNow = true;
                    for (const auto& proj : projectiles) {
                        SDL_Rect projRect = {static_cast<int>(proj.x), static_cast<int>(proj.y), proj.w, proj.h};
                        if (SDL_HasIntersection(&projRect, &fruitRects[i])) {
                            canShootNow = false;
                            break;
                        }
                    }
                    if (canShootNow) {
                        float px = fruitRects[i].x + fruitRects[i].w / 2.0f - 10;
                        float py = fruitRects[i].y + fruitRects[i].h / 2.0f - 10;
                        float dx = (fruitRects[targetIdx].x + fruitRects[targetIdx].w / 2.0f) - (fruitRects[i].x + fruitRects[i].w / 2.0f);
                        float dy = (fruitRects[targetIdx].y + fruitRects[targetIdx].h / 2.0f) - (fruitRects[i].y + fruitRects[i].h / 2.0f);
                        float len = std::sqrt(dx * dx + dy * dy);
                        if (len > 0) { dx /= len; dy /= len; }
                        float speed = 10.0f;
                        projectiles.push_back({px, py, dx * speed, dy * speed, 20, 20, fruitColors[i], i});
                        fruitProjectiles[i]--;
                    }
                }
            }
        }
        // Player movement
        handleAppleMovement(fruitRects[0]);
        // AI movement
        if (useAI) {
            for (int i = 1; i < numAIs + 1; ++i) {
                handleAIMovement(fruitRects[i], fruitTargets[i]);
            }
        } else {
            handlePearMovement(fruitRects[1]);
        }
        // Update projectiles
        for (auto& proj : projectiles) {
            proj.x += proj.vx;
            proj.y += proj.vy;
        }
        // Check projectile collision and remove on hit
        projectiles.erase(
            std::remove_if(projectiles.begin(), projectiles.end(), [&](const Projectile& proj) {
                SDL_Rect projRect = {static_cast<int>(proj.x), static_cast<int>(proj.y), proj.w, proj.h};
                // Any fruit's projectiles can hit any other fruit (not self)
                if (proj.owner >= 0 && proj.owner < numAIs + 1) {
                    for (int i = 0; i < numAIs + 1; ++i) {
                        if (i == proj.owner) continue;
                        if (SDL_HasIntersection(&projRect, &fruitRects[i])) {
                            fruitRects[i].w = std::max(20, fruitRects[i].w - 10);
                            fruitRects[i].h = std::max(20, fruitRects[i].h - 10);
                            keepInBounds(fruitRects[i]);
                            return true;
                        }
                    }
                }
                // Remove if out of bounds
                return proj.x < 0 || proj.y < 0 || proj.x + proj.w > WINDOW_WIDTH || proj.y + proj.h > WINDOW_HEIGHT;
            }),
            projectiles.end()
        );
        // --- Rendering ---
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        for (int i = 0; i < numAIs + 1; ++i) renderRect(renderer, fruitTargets[i], fruitColors[i]);
        // Determine transparency for overlap
        Uint8 alpha[5] = {255, 255, 255, 255, 255};
        for (int i = 0; i < numAIs + 1; ++i) {
            for (int j = 0; j < numAIs + 1; ++j) {
                if (i != j && SDL_HasIntersection(&fruitRects[i], &fruitRects[j])) alpha[i] = 128;
            }
            if (SDL_HasIntersection(&fruitRects[i], &fruitTargets[i])) alpha[i] = 128;
        }
        for (int i = 0; i < numAIs + 1; ++i) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, fruitColors[i].r, fruitColors[i].g, fruitColors[i].b, alpha[i]);
            SDL_RenderFillRect(renderer, &fruitRects[i]);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, alpha[i]);
            SDL_RenderDrawRect(renderer, &fruitRects[i]);
        }
        for (const auto& proj : projectiles) renderProjectile(renderer, proj);
        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    // --- Cleanup ---
    std::cout << "Game ended. Thanks for playing!\n";
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}