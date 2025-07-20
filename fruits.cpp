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

// === Global Constants ===
double taxRate = 0.025;

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
SDL_Rect fruitRects[5] = {
	{rand() % (WINDOW_WIDTH - 50), rand() % (WINDOW_HEIGHT - 50), 50, 50}, // Apple
	{rand() % (WINDOW_WIDTH - 50), rand() % (WINDOW_HEIGHT - 50), 50, 50}, // Pear
	{rand() % (WINDOW_WIDTH - 50), rand() % (WINDOW_HEIGHT - 50), 50, 50}, // Lemon
	{rand() % (WINDOW_WIDTH - 50), rand() % (WINDOW_HEIGHT - 50), 50, 50}, // Blueberry
	{rand() % (WINDOW_WIDTH - 50), rand() % (WINDOW_HEIGHT - 50), 50, 50}  // Orange
};

void initFruitRects() {
	for (int i = 0; i <= 5; i++) {
		fruitRects[i] = {rand() % (WINDOW_WIDTH - 50), rand() % (WINDOW_HEIGHT - 50), 50, 50}; // Apple
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
void initFruitTargets() {
	for (int i = 0; i < 5; ++i) {
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

void placeBets(const std::string& playerName, int numFruits) {

	std::cout << "Hello " << playerName << "! Place your bets on the fruits.\n";
    
	// For loop for multiple bets
    bool betting = true;
    int betIndex = -1; // Index of the fruit the player bets on
    
	// Print available fruits to bet on
	std::cout << "Available fruits to bet on:\n";
    for (int i = 0; i < numFruits; ++i) {
        std::cout << i + 1 << ": " << fruitNames[i] << "\n";
    }

	// Get the player's bet
    std::cout << "Enter the number of the fruit you want to bet on (1-" << numFruits << "): ";
    std::string betInput;
    std::getline(std::cin, betInput);
    if (!betInput.empty() && std::isdigit(betInput[0])) {
        betIndex = std::stoi(betInput) - 1; // Convert to zero-based index
        if (betIndex < 0 || betIndex >= numFruits) {
            std::cout << "Invalid bet. No bet placed.\n";
            return;
        }
        std::cout << "You're betting on: " << fruitNames[betIndex] << "\n";
    } else {
        std::cout << "Invalid input. No bets allowed.\n";
        return;
    }

    // Loop for placing bets
    while (betting) {
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
            bets.push_back(newBet); // Use global bets, do not redeclare
            std::cout << "Bet placed successfully!\n";
            std::cout << "You can continue betting by pressing any key or exit by typing 'exit'.\n";
            std::string continueInput;
            std::getline(std::cin, continueInput);
            if (continueInput == "exit" || continueInput == "Exit") {
                betting = false; // Exit betting loop
                std::cout << "Betting finished. Good luck!\n";
                return;
            } else {
                std::cout << "Continuing betting...\n";
            }
        } catch (...) {
            std::cout << "Invalid input. Please enter a valid number.\n";
        }
    }
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

void calculatePayouts(int numFruits, int winningFruitIndex) {
	double totalBetAmount = 0.0;
	for (const auto& bet : bets) {
		totalBetAmount += bet.amount;
	}
	
	if (totalBetAmount == 0) {
		std::cout << "No bets placed. No payouts to calculate.\n";
		return;
	}

	double taxAmount = totalBetAmount * taxRate;


	double payoutMultiplier = static_cast<double>(numFruits) / 2.0; // Payout multiplier based on number of fruits
	double payout = (totalBetAmount - taxAmount) * payoutMultiplier;

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

	payout = (payout / totalWinningBets) * totalBetAmount; // Distribute payout based on winning bets
	// Print payout details
	// Print final payout calculation
	std::cout << "=== Payout Calculation ===\n";
	std::cout << "Total bets together (The pot): " << totalBetAmount << " euros\n";
	std::cout << "Tax amount (2.5%): " << taxAmount << " euros\n";
	std::cout << "Total payout for " << fruitNames[winningFruitIndex] << ": " << payout << " euros after tax.\n";
	std::cout << "=========================\n";
	// Print betting results for each player
	for (const auto& bet : bets) {
		if (bet.fruitIndex == winningFruitIndex) {
			double playerPayout = (bet.amount / totalWinningBets) * payout;
			std::cout << bet.playerName << " wins " << playerPayout << " euros by betting on " << fruitNames[winningFruitIndex] << ".\n";
		} else {
			std::cout << bet.playerName << " lost their bet of " << bet.amount << " euros on " << fruitNames[bet.fruitIndex] << ".\n";
		}
	}
}

void checkWinCondition(SDL_Rect* fruits, int numFruits, bool& running, bool aiVsAiMode) {
    for (int i = 0; i < numFruits; ++i) {
        // Check if player1 wins (covers whole screen)
        if (fruits[i].w >= WINDOW_WIDTH && fruits[i].h >= WINDOW_HEIGHT) {
            std::cout << "\n" << fruitNames[i] << " won!\n";
            std::cout << fruitNames[i] << " covered the entire screen!\n";
			std::cout << "Game Over!\n";
			
			if (aiVsAiMode) {
				std::cout << "The winning fruit was: " << fruitNames[i] << ".\n";
				std::cout << "Calculating payouts...\n";
            }
	
            // Thank you message
            std::cout << "=========================\n";
            std::cout << "Thank you for playing! Hope you enjoyed the game!\n";
            std::cout << "=========================\n";
        
            // End the game
            std::cout << "Game Over!\n";
            std::cout << "Press any key to exit...\n";
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
    bool aiVsAiMode = false; // Initialize global flag
    int betIndex = -1;
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
        } else if (arg == "--aivsai") {
            aiVsAiMode = true;
            useAI = true;
            aiCanShoot = true;
            if (argc > 2) {
                try {
                    numAIs = std::stoi(argv[2]);
                    if (numAIs < 2) numAIs = 2;
                    if (numAIs > 5) numAIs = 5;
                } catch (...) {
                    numAIs = 2;
                }
            } else {
                numAIs = 2;
            }
            std::cout << "Starting in AI vs AI mode (" << numAIs << " AI fruits)\n";
			
			// Initialize fruit targets for AI vs AI mode
			initFruitTargets();

			// Betting system for AI vs AI mode
			std::cout << "Welcome to the SECRET Betting System!\n";
			std::cout << "You can place bets on the fruits in AI vs AI mode.\n";

			// Get the number of players betting
			std::cout << "How many players are betting? (1-10): ";
			int numPlayers;
			std::string input;
			std::getline(std::cin, input);
			try {
				numPlayers = std::stoi(input);
				if (numPlayers < 1 || numPlayers > 10) {
					std::cout << "Invalid number of players. Defaulting to 2 players.\n";
					numPlayers = 2;
				}
			} catch (...) {
				std::cout << "Invalid input. Defaulting to 2 players.\n";
				numPlayers = 2;
			}

			// For each player, place bets
			for (int i = 1; i <= numPlayers; ++i) {
				std::cout << "Player " << i << ", please enter your name to place your bets: ";
				std::string playerName;
				std::getline(std::cin, playerName);
				if (playerName.empty()) {
					playerName = "Player " + std::to_string(i); // Default name if none entered
				}
				placeBets(playerName, numAIs + 1); // Place bets for each player
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
    }

    // Print controls
    printControls();

	// Init fruit rectangles
	initFruitRects();

	// Init fruitTargets
	initFruitTargets();

	// Init projectiles
    std::vector<Projectile> projectiles;

	bool running = true;
    bool appleShootPressed = false;
	SDL_Event event;

    // === Main Loop ===
	while (running) {
        // --- Event Handling ---
		if (!aiVsAiMode) {
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
		}

        // --- Game Logic ---
        for (int i = 0; i < numAIs + 1; ++i) keepInBounds(fruitRects[i]);
        // Check win condition for all fruits
        checkWinCondition(fruitRects, numAIs + 1, running, aiVsAiMode);
        if (!running) break;
        // Check if each fruit eats its target
        for (int i = 0; i < numAIs + 1; ++i) {
            int prevW = fruitRects[i].w, prevH = fruitRects[i].h;
            checkEatFruit(fruitRects[i], fruitTargets[i]);
            if (fruitRects[i].w > prevW || fruitRects[i].h > prevH) {
                fruitProjectiles[i] = 5;
                std::cout << fruitNames[i] << " ate its fruit! Projectiles reset to 5\n";
            }
        }
        // Apple shooting (mouse click - only in AI mode)
        if (!aiVsAiMode && appleShootPressed && fruitProjectiles[0] > 0 && useAI) {
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
            for (int i = (aiVsAiMode ? 0 : 1); i < numAIs + 1; ++i) {
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
        if (!aiVsAiMode) handleAppleMovement(fruitRects[0]);
        // AI movement
        if (useAI) {
            for (int i = (aiVsAiMode ? 0 : 1); i < numAIs + 1; ++i) {
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
    if (aiVsAiMode) {
		// Calculate payouts if in AI vs AI mode
		int winningFruitIndex = -1;
		for (int i = 0; i < numAIs + 1; ++i) {
			if (fruitRects[i].w >= WINDOW_WIDTH && fruitRects[i].h >= WINDOW_HEIGHT) {
				winningFruitIndex = i;
				break;
			}
		}
		if (winningFruitIndex != -1) {
			std::cout << "The winning fruit was: " << fruitNames[winningFruitIndex] << ".\n";
			std::cout << "Calculating payouts...\n";
			calculatePayouts(numAIs + 1, winningFruitIndex);
		} else {
			std::cout << "No fruit won. No payouts to calculate.\n";
		}
    }

    std::cout << "Game ended. Thanks for playing!\n";
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
	
	return 0;
}
