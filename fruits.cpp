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
#include <string>
#include <cctype>

// === Constants ===
constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;
constexpr int PLAYER_SPEED = 5;
constexpr int PROJECTILE_SPEED = 10;
constexpr int FRUIT_GROWTH = 10;
constexpr int FRUIT_MIN_SIZE = 20;
constexpr int FRUIT_TARGET_SIZE = 20;
constexpr int FRUIT_START_SIZE = 50;
constexpr int PROJECTILE_SIZE = 20;
constexpr int MAX_PROJECTILES = 3;

// === Structs ===
struct Projectile {
    float x, y;
    float vx, vy;
    int w, h;
    SDL_Color color;
    int owner; // 0 = player1, 1 = player2, 2 = lemonAI, 3 = blueberryAI, 4 = orangeAI
};

struct Bet {
	// Structure to hold player bets
	std::string playerName;		// Name of the player placing the bet
	int fruitIndex; 			// Index of the fruit the player bets on, multiple bets allowed
	double amount; 				// Amount of the bet in euros
};

std::vector<Bet> bets;

struct Fruit {
	SDL_Rect rect;
	SDL_Color color;
	int projectiles; // Number of projectiles left
};

// === Fruit Colors ===
const SDL_Color appleColor = {255, 0, 0, 255};          // Apple (Red)
const SDL_Color pearColor = {0, 255, 0, 255};           // Pear (Green)
const SDL_Color lemonColor = {255, 255, 0, 255};        // Lemon (Yellow)
const SDL_Color blueberryColor = {0, 128, 255, 255};    // Blueberry (Blue)
const SDL_Color orangeColor = {255, 140, 0, 255};       // Orange (Orange)

// === Fruit Names ===
const std::string fruitNames[5] = {"Apple", "Pear", "Lemon", "Blueberry", "Orange"};

// === Fruit Rectangles ===
SDL_Rect fruitRects[5]; // Only initialize as needed

void initFruitRects(int numFruits) {
    for (int i = 0; i < numFruits; i++) {
        fruitRects[i] = {rand() % (WINDOW_WIDTH - 50), rand() % (WINDOW_HEIGHT - 50), 50, 50};
    }
}

// === Fruit Colors Array ===
SDL_Color fruitColors[5] = {
	{255, 0, 0, 255},      // Apple (Red)
	{0, 255, 0, 255},      // Pear (Green)
	{255, 255, 0, 255},    // Lemon (Yellow)
	{0, 128, 255, 255},    // Blueberry (Blue)
	{255, 140, 0, 255}     // Orange
};

// === Fruit Projectiles Array ===
int fruitProjectiles[5] = {3, 3, 3, 3, 3}; // Each fruit starts with 3 projectiles

// === Fruit Targets ===
SDL_Rect fruitTargets[5];

// Initialize fruit targets
void initFruitTargets(int numFruits) {
    for (int i = 0; i < numFruits; ++i) {
        fruitTargets[i] = {rand() % (WINDOW_WIDTH - 20), rand() % (WINDOW_HEIGHT - 20), 20, 20};
    }
}

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
// === Movement Handling ===
// keys: {up, down, left, right}
void handlePlayerMovement(SDL_Rect& player, const std::vector<SDL_Scancode>& keys) {
    const Uint8* state = SDL_GetKeyboardState(NULL);
    if (state[keys[0]]) player.y -= PLAYER_SPEED;
    if (state[keys[1]]) player.y += PLAYER_SPEED;
    if (state[keys[2]]) player.x -= PLAYER_SPEED;
    if (state[keys[3]]) player.x += PLAYER_SPEED;
}

// Generalized AI movement towards a target
void handleAIMovement(SDL_Rect& ai, const SDL_Rect& target) {
    if (ai.x < target.x)      ai.x += PLAYER_SPEED;
    else if (ai.x > target.x) ai.x -= PLAYER_SPEED;
    if (ai.y < target.y)      ai.y += PLAYER_SPEED;
    else if (ai.y > target.y) ai.y -= PLAYER_SPEED;
}

// === Utility Functions ===
void keepInBounds(SDL_Rect& rect) {
    if (rect.x < 0) rect.x = 0;
    if (rect.y < 0) rect.y = 0;
    if (rect.x + rect.w > WINDOW_WIDTH)  rect.x = WINDOW_WIDTH - rect.w;
    if (rect.y + rect.h > WINDOW_HEIGHT) rect.y = WINDOW_HEIGHT - rect.h;
}

void checkEatFruit(SDL_Rect& player, SDL_Rect& fruit, bool screensaverMode) {
	if (SDL_HasIntersection(&player, &fruit) && screensaverMode) {
		// Generate a new random position for the fruit using srand()
		fruit.x = rand() % (WINDOW_WIDTH - 50);
		fruit.y = rand() % (WINDOW_HEIGHT - 50);
        keepInBounds(player);
    } else if (SDL_HasIntersection(&player, &fruit)) {
		fruit.x = rand() % (WINDOW_WIDTH - 50);
		fruit.y = rand() % (WINDOW_HEIGHT - 50);
		player.w += FRUIT_GROWTH;
		player.h += FRUIT_GROWTH;
        keepInBounds(player);
    }
}

// === User-Friendly Output Functions ===
void printWelcome() {
    std::cout << "\n==================================================\n";
    std::cout << "            Welcome to Fruits!\n";
    std::cout << "         An agar.io-like game with fruits.\n";
    std::cout << "==================================================\n";
}

void printHelp() {
    std::cout << "\n================= GAME CONTROLS ==================\n";
    std::cout << "Apple (Red):   WASD to move, Mouse to shoot (AI mode only)\n";
    std::cout << "Pear (Green):  Arrow keys to move\n";
    std::cout << "T:             Toggle AI shooting (AI mode only)\n";
    std::cout << "H:             Show this help message\n";
    std::cout << "ESC:           Quit game\n";
    std::cout << "==================================================\n\n";
}

void placeBets(const std::string& playerName, int numFruits) {
    std::cout << "Hello " << playerName << "! Place your bets on the fruits.\n";
    std::vector<bool> alreadyBet(numFruits, false);
    bool betting = true;
    while (betting) {
        // Print available fruits to bet on
        std::cout << "Available fruits to bet on:\n";
        for (int i = 0; i < numFruits; ++i) {
            if (!alreadyBet[i])
                std::cout << i + 1 << ": " << fruitNames[i] << "\n";
        }
        std::cout << "Enter the number of the fruit you want to bet on (1-" << numFruits << ", or 0 to finish): ";
        std::string betInput;
        std::getline(std::cin, betInput);
        if (!betInput.empty() && std::isdigit(betInput[0])) {
            int betIndex = std::stoi(betInput) - 1; // Convert to zero-based index
            if (betIndex == -1) {
                std::cout << "Finished betting for " << playerName << ".\n";
                break;
            }
            if (betIndex < 0 || betIndex >= numFruits || alreadyBet[betIndex]) {
                std::cout << "Invalid or duplicate bet. Try again.\n";
                continue;
            }
            std::cout << "You're betting on: " << fruitNames[betIndex] << "\n";
            std::cout << "Enter the amount you want to bet on " << fruitNames[betIndex] << ": ";
            std::string amountInput;
            std::getline(std::cin, amountInput);
            try {
                double amount = std::stod(amountInput);
                if (amount <= 0) {
                    std::cout << "Invalid amount. Please enter a positive number.\n";
                    continue;
                }
                std::cout << playerName << " placed a bet of " << amount << " euros on " << fruitNames[betIndex] << ".\n";
                Bet newBet = {playerName, betIndex, amount};
                bets.push_back(newBet);
                alreadyBet[betIndex] = true;
            } catch (...) {
                std::cout << "Invalid input. Please enter a valid number.\n";
                continue;
            }
            // Ask if the player wants to bet on another fruit
            bool validChoice = false;
            while (!validChoice) {
                std::cout << "Do you want to place another bet? (y/n): ";
                std::string anotherInput;
                std::getline(std::cin, anotherInput);
                if (anotherInput == "n" || anotherInput == "N") {
                    betting = false;
                    validChoice = true;
                } else if (anotherInput == "y" || anotherInput == "Y") {
                    validChoice = true;
                } else {
                    std::cout << "Please enter 'y' or 'n'.\n";
                }
            }
        } else {
            std::cout << "Invalid input. Try again.\n";
        }
    }
}

void summarizeBets() {
    std::cout << "\n===================== BETS SUMMARY =======================\n";
    if (bets.empty()) {
        std::cout << "No bets placed.\n";
    } else {
        for (const auto& bet : bets) {
            std::cout << bet.playerName << " bet " << bet.amount << " euros on " << fruitNames[bet.fruitIndex] << "\n";
        }
    }
    std::cout << "==========================================================\n";
}

void calculatePayouts(int numFruits, int winningFruitIndex) {
    double totalBetAmount = 0.0;
    for (const auto& bet : bets) {
        totalBetAmount += bet.amount;
    }
    
    if (totalBetAmount == 0) {
        std::cout << "No bets placed. No payouts to calculate.\n";
        return;
    }

    // Calculate taxes
    double payoutPool = totalBetAmount;

    // Total payout for each player who bet on the winning fruit
    double totalWinningBets = 0.0;
    for (const auto& bet : bets) {
        if (bet.fruitIndex == winningFruitIndex) {
            totalWinningBets += bet.amount;
        }
    }

    if (totalWinningBets == 0) {
        std::cout << "No bets placed on the winning fruit: " << fruitNames[winningFruitIndex] << ". No payouts to calculate.\n";
        return;
    }

    // Print betting summary before payout calculation
    summarizeBets();

    std::cout << "\n=================== PAYOUT CALCULATION ===================\n";
    std::cout << "Total bets together (The pot): " << totalBetAmount << " euros\n";
    std::cout << "Total payout for " << fruitNames[winningFruitIndex] << ": " << payoutPool << ".\n";
    std::cout << "==========================================================\n";
    for (const auto& bet : bets) {
        if (bet.fruitIndex == winningFruitIndex) {
            double playerPayout = (bet.amount / totalWinningBets) * payoutPool;
            std::cout << bet.playerName << " wins " << playerPayout << " euros by betting on " << fruitNames[winningFruitIndex] << ".\n";
        } else {
            std::cout << bet.playerName << " lost their bet of " << bet.amount << " euros on " << fruitNames[bet.fruitIndex] << ".\n";
        }
    }
    std::cout << "==========================================================\n";
}

void checkWinCondition(SDL_Rect* fruits, int numFruits, bool& running, bool aiVsAiMode) {
    for (int i = 0; i < numFruits; ++i) {
        // Check if player1 wins (covers whole screen)
        if (fruits[i].w >= WINDOW_WIDTH && fruits[i].h >= WINDOW_HEIGHT) {
            std::cout << "\n" << fruitNames[i] << " won!\n";
            std::cout << fruitNames[i] << " covered the entire screen!\n";
		}
	}

	if (aiVsAiMode) {
		int winningFruitIndex = -1;
		for (int i = 0; i < numFruits; ++i) { // For loop for all fruits
			if (fruitRects[i].w >= WINDOW_WIDTH && fruitRects[i].h >= WINDOW_HEIGHT) { // Check if any fruit covers the entire screen
				winningFruitIndex = i; // Set "winningFruit" with its index.
				break;
			}
		}
		if (winningFruitIndex != -1) { // If "winningFruit" has been set (is not -1), Calculate payouts.
			std::cout << "The winning fruit was: " << fruitNames[winningFruitIndex] << ".\n";
			// If betting is enabled, calculate payouts
			if (!bets.empty()) {
				std::cout << "Calculating payouts...\n";
				calculatePayouts(numFruits, winningFruitIndex);
    
				// Thank you message
		    	std::cout << "==================================================\n";
    			std::cout << "Thank you for playing! Hope you enjoyed the game!\n";
				std::cout << "==================================================\n";
       
	    		// End the game
	    		std::cout << "Press any key to exit...\n";
				running = false;
			}
		}
	}
}

// === Projectile Handling ===
// Helper to create a projectile
void createProjectile(std::vector<Projectile>& projectiles, int owner, float x, float y, float dx, float dy, SDL_Color color) {
    float len = std::sqrt(dx * dx + dy * dy);
    if (len > 0) { dx /= len; dy /= len; }
    projectiles.push_back({x, y, dx * PROJECTILE_SPEED, dy * PROJECTILE_SPEED, PROJECTILE_SIZE, PROJECTILE_SIZE, color, owner});
}

// Handle projectile collisions and shrinking
void handleProjectileCollisions(std::vector<Projectile>& projectiles, SDL_Rect* fruitRects, int numFruits, bool aiVsAiMode) {
    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(), [&](const Projectile& proj) {
            SDL_Rect projRect = {static_cast<int>(proj.x), static_cast<int>(proj.y), proj.w, proj.h};
            for (int i = 0; i < numFruits; ++i) {
                if (i == proj.owner) continue;
                if (SDL_HasIntersection(&projRect, &fruitRects[i])) {
                    int shrink = aiVsAiMode ? 2 : 5;
                    fruitRects[i].w = std::max(FRUIT_MIN_SIZE, fruitRects[i].w - shrink);
                    fruitRects[i].h = std::max(FRUIT_MIN_SIZE, fruitRects[i].h - shrink);
                    keepInBounds(fruitRects[i]);
                    return true;
                }
            }
            return proj.x < 0 || proj.y < 0 || proj.x + proj.w > WINDOW_WIDTH || proj.y + proj.h > WINDOW_HEIGHT;
        }),
        projectiles.end()
    );
}

// === Main Game Loop Modularization ===
// Event handling
void handleEvents(bool& running, bool& appleShootPressed, bool& aiCanShoot, bool aiVsAiMode, bool useAI, int* fruitProjectiles) {
    SDL_Event event;
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
                case SDLK_h:
                    printHelp();
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
}

// Game logic update
void updateGameLogic(bool& running, bool aiVsAiMode, bool useAI, bool aiCanShoot, bool& appleShootPressed, int numFruits, int* fruitProjectiles, SDL_Rect* fruitRects, SDL_Rect* fruitTargets, std::vector<Projectile>& projectiles, bool screensaverMode) {
    for (int i = 0; i < numFruits; ++i) keepInBounds(fruitRects[i]);
    checkWinCondition(fruitRects, numFruits, running, aiVsAiMode);
    if (!running) return;
    for (int i = 0; i < numFruits; ++i) {
        int prevW = fruitRects[i].w, prevH = fruitRects[i].h;
        checkEatFruit(fruitRects[i], fruitTargets[i], screensaverMode);
        if (fruitRects[i].w > prevW || fruitRects[i].h > prevH) {
            fruitProjectiles[i] = MAX_PROJECTILES;
        }
    }
    // Apple shooting (mouse click - only in AI mode)
    if (!aiVsAiMode && appleShootPressed && fruitProjectiles[0] > 0 && useAI) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        float px = fruitRects[0].x + fruitRects[0].w / 2.0f - PROJECTILE_SIZE / 2.0f;
        float py = fruitRects[0].y + fruitRects[0].h / 2.0f - PROJECTILE_SIZE / 2.0f;
        float dx = mouseX - (fruitRects[0].x + fruitRects[0].w / 2.0f);
        float dy = mouseY - (fruitRects[0].y + fruitRects[0].h / 2.0f);
        createProjectile(projectiles, 0, px, py, dx, dy, fruitColors[0]);
        fruitProjectiles[0]--;
        appleShootPressed = false;
    }
    // AI shooting
    if (useAI && aiCanShoot) {
        for (int i = (aiVsAiMode ? 0 : 1); i < numFruits; ++i) {
            if (fruitProjectiles[i] > 0) {
                float minDist = std::numeric_limits<float>::max();
                int targetIdx = 0;
                for (int j = 0; j < numFruits; ++j) {
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
                    float px = fruitRects[i].x + fruitRects[i].w / 2.0f - PROJECTILE_SIZE / 2.0f;
                    float py = fruitRects[i].y + fruitRects[i].h / 2.0f - PROJECTILE_SIZE / 2.0f;
                    float dx = (fruitRects[targetIdx].x + fruitRects[targetIdx].w / 2.0f) - (fruitRects[i].x + fruitRects[i].w / 2.0f);
                    float dy = (fruitRects[targetIdx].y + fruitRects[targetIdx].h / 2.0f) - (fruitRects[i].y + fruitRects[i].h / 2.0f);
                    createProjectile(projectiles, i, px, py, dx, dy, fruitColors[i]);
                    fruitProjectiles[i]--;
                }
            }
        }
    }
    // Player movement
    if (!aiVsAiMode) handlePlayerMovement(fruitRects[0], {SDL_SCANCODE_W, SDL_SCANCODE_S, SDL_SCANCODE_A, SDL_SCANCODE_D});
    if (useAI) {
        for (int i = (aiVsAiMode ? 0 : 1); i < numFruits; ++i) {
            handleAIMovement(fruitRects[i], fruitTargets[i]);
        }
    } else if (numFruits > 1) {
        handlePlayerMovement(fruitRects[1], {SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT});
    }
    // Update projectiles
    for (auto& proj : projectiles) {
        proj.x += proj.vx;
        proj.y += proj.vy;
    }
    // Handle projectile collisions
    handleProjectileCollisions(projectiles, fruitRects, numFruits, aiVsAiMode);
}

// Rendering
void renderGame(SDL_Renderer* renderer, int numFruits, SDL_Rect* fruitRects, SDL_Rect* fruitTargets, SDL_Color* fruitColors, const std::vector<Projectile>& projectiles) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    for (int i = 0; i < numFruits; ++i) renderRect(renderer, fruitTargets[i], fruitColors[i]);
    Uint8 alpha[5] = {255, 255, 255, 255, 255};
    for (int i = 0; i < numFruits; ++i) {
        for (int j = 0; j < numFruits; ++j) {
            if (i != j && SDL_HasIntersection(&fruitRects[i], &fruitRects[j])) alpha[i] = 128;
        }
        if (SDL_HasIntersection(&fruitRects[i], &fruitTargets[i])) alpha[i] = 128;
    }
    for (int i = 0; i < numFruits; ++i) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, fruitColors[i].r, fruitColors[i].g, fruitColors[i].b, alpha[i]);
        SDL_RenderFillRect(renderer, &fruitRects[i]);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, alpha[i]);
        SDL_RenderDrawRect(renderer, &fruitRects[i]);
    }
    for (const auto& proj : projectiles) renderProjectile(renderer, proj);
    SDL_RenderPresent(renderer);
}

// === Main Game ===
int main(int argc, char* argv[]) {

	// Seed random number generator, this generates a random seed based on the current time, and rand() will work using this seed.
	std::srand(static_cast<unsigned int>(std::time(nullptr)));

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
    int numFruits = 2; // Default to 2 fruits (Apple, Pear)
    bool aiVsAiMode = false; // Initialize global flag
	bool screensaverMode = false;
    int betIndex = -1;
    if (argc > 1) {
        std::string arg = argv[1];
        bool enableBets = false;
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
            numFruits = numAIs + 1; // Apple + N AIs
            std::cout << "Starting in PvAI mode (Apple vs " << numAIs << " AI opponent(s))\n";
        } else if (arg == "--aivsai") {
            aiVsAiMode = true;
            useAI = true;
            aiCanShoot = true; // Always enable AI shooting in AI vs AI mode
            bool enableBets = false;
            int numPlayers = 2; // Default number of betting players
            int nFruits = 2; // Default number of AI fruits
            // Parse N from command line
            if (argc > 2) {
                try {
                    nFruits = std::stoi(argv[2]);
                    if (nFruits < 2) nFruits = 2;
                    if (nFruits > 5) nFruits = 5;
                } catch (...) {
                    nFruits = 2;
                }
            }
            // Check for --bets flag
	        for (int i = 3; i < argc; ++i) {
                if (std::string(argv[i]) == "--bets") {
                    enableBets = true;
                    break;
                }
				if (std::string(argv[i]) == "--screensaver") {
					screensaverMode = true;
					break;
				}
            }
            numAIs = nFruits;
            numFruits = nFruits;
            std::cout << "Starting in AI vs AI mode (" << numAIs << " AI fruits)" << (enableBets ? " with BETTING" : " WITHOUT betting") << "\n";
            initFruitTargets(numFruits);
            if (enableBets) {
                // Prompt for number of betting players
                std::cout << "How many players are betting? (1-10): ";
                std::string input;
                std::getline(std::cin, input);
                try {
                    int inputPlayers = std::stoi(input);
                    if (inputPlayers < 1 || inputPlayers > 10) {
                        std::cout << "Invalid number of players. Defaulting to 2 players.\n";
                        numPlayers = 2;
                    } else {
                        numPlayers = inputPlayers;
                    }
                } catch (...) {
                    std::cout << "Invalid input. Defaulting to 2 players.\n";
                    numPlayers = 2;
                }
                std::cout << "Welcome to the SECRET Betting System!\n";
                std::cout << "You can place bets on the " << numFruits << " fruits in AI vs AI mode.\n";
                for (int i = 1; i <= numPlayers; ++i) {
                    std::cout << "Player " << i << ", please enter your name to place your bets: ";
                    std::string playerName;
                    std::getline(std::cin, playerName);
                    if (playerName.empty()) {
                        playerName = "Player " + std::to_string(i);
                    }
                    placeBets(playerName, numFruits);
                }
            }
			if (screensaverMode) {
				aiCanShoot = false;
			}
        } else {
            std::cout << "Usage: " << argv[0] << " [--ai N | --aivsai N]\n";
            std::cout << "  --ai N: PvAI mode (Apple vs N AIs, N=1-4)\n";
            std::cout << "  --aivsai N: AI vs AI mode (N=2-5)\n";
            std::cout << "  No arguments: PvP mode (Apple vs Pear)\n";
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 1;
        }
    } else {
        std::cout << "Starting in PvP mode (Apple vs Pear)\n";
        numAIs = 1;

        numFruits = 2;
    }

    // Print controls
    printWelcome();
    printHelp();

    // Init fruit rectangles and targets for the correct number of fruits
    initFruitRects(numFruits);
    initFruitTargets(numFruits);

    // Init projectiles
    std::vector<Projectile> projectiles;
    for (int i = 0; i < numFruits; ++i) fruitProjectiles[i] = 3;

	bool running = true;
    bool appleShootPressed = false;
	SDL_Event event;

    // === Main Loop ===
	while (running) {
        // --- Event Handling ---
		if (!aiVsAiMode) {
			handleEvents(running, appleShootPressed, aiCanShoot, aiVsAiMode, useAI, fruitProjectiles);
		} else if (aiVsAiMode) {
			handleEvents(running, appleShootPressed, aiCanShoot, aiVsAiMode, useAI, fruitProjectiles);
		}

        // --- Game Logic ---
		updateGameLogic(running, aiVsAiMode, useAI, aiCanShoot, appleShootPressed, numFruits, fruitProjectiles, fruitRects, fruitTargets, projectiles, screensaverMode);
        if (!running) break;

        // --- Rendering ---
		renderGame(renderer, numFruits, fruitRects, fruitTargets, fruitColors, projectiles);
        SDL_Delay(16); // ~60 FPS
    }

    std::cout << "Game ended. Thanks for playing!\n";
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
	
	return 0;
}
