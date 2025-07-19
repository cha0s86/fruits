// This is a simple agar.io-like game using SDL2
// It is a basic implementation of a game where, players are moving around the screen and eating smaller fruits will make the player grow
// To compile this code, you need to have SDL2 installed and link against the SDL2 library.
#include <SDL2/SDL.h>
#include <iostream>

// SDL Constants for window dimensions
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// Function to render the fruit object
void renderFruits(SDL_Renderer* renderer, const SDL_Rect& fruit, SDL_Color color) {
	// Set the color for the fruit
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
	// Render the fruit as a filled rectangle
	SDL_RenderFillRect(renderer, &fruit);
	// Set the color for the border
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White color
	// Draw a border around the fruit
	SDL_RenderDrawRect(renderer, &fruit);
}

// Function to render the player
void renderPlayer(SDL_Renderer* renderer, const SDL_Rect& player) {
	// Set the color for the player
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red color
	// Render the player as a filled rectangle
	SDL_RenderFillRect(renderer, &player);
	// Set the color for the border
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White color
	// Draw a border around the player
	SDL_RenderDrawRect(renderer, &player);
}

void renderPlayer2(SDL_Renderer* renderer, const SDL_Rect& player) {
	// Set the color for the player
	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Green color
	// Render the player as a filled rectangle
	SDL_RenderFillRect(renderer, &player);
	// Set the color for the border
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White color
	// Draw a border around the player
	SDL_RenderDrawRect(renderer, &player);
}
	
// Function to handle player1 input
void handlePlayerInput(SDL_Rect& player) {
	const Uint8* state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_UP]) {
		player.y -= 5; // Move up
	}
	if (state[SDL_SCANCODE_DOWN]) {
		player.y += 5; // Move down
	}
	if (state[SDL_SCANCODE_LEFT]) {
		player.x -= 5; // Move left
	}
	if (state[SDL_SCANCODE_RIGHT]) {
		player.x += 5; // Move right
	}
}

// Function to handle player2 input
void handlePlayer2Input(SDL_Rect& player) {
	const Uint8* state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_W]) {
		player.y -= 5; // Move up
	}
	if (state[SDL_SCANCODE_S]) {
		player.y += 5; // Move down
	}
	if (state[SDL_SCANCODE_A]) {
		player.x -= 5; // Move left
	}
	if (state[SDL_SCANCODE_D]) {
		player.x += 5; // Move right
	}
}

void checkForOutOfBounds(SDL_Rect& player) {
	// Ensure player does not exceed window bounds
	if (player.x < 0) player.x = 0;
	if (player.y < 0) player.y = 0;
	if (player.x + player.w > WINDOW_WIDTH) player.x = WINDOW_WIDTH - player.w;
	if (player.y + player.h > WINDOW_HEIGHT) player.y = WINDOW_HEIGHT - player.h;
}

void checkForCollisionWithFruits(SDL_Rect& player, SDL_Rect& fruit) {
	// Check for collision between player and fruit
	if (SDL_HasIntersection(&player, &fruit)) {
		// Reset fruit position to a new random location
		fruit.x = rand() % (WINDOW_WIDTH - 50);
		fruit.y = rand() % (WINDOW_HEIGHT - 50);
		// Increase player size
		player.w += 10;
		player.h += 10;
		// Ensure player does not exceed window bounds
		checkForOutOfBounds(player);
	}
}

// Function to initialize SDL and create a window and renderer
int main(int argc, char* argv[]) {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Fruits! - Agar.io-like Game",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
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

	// Initialize player and apple positions
	// Random positioning for player and fruits
	srand(static_cast<unsigned int>(time(0))); // Seed for random number generation

	// Player rects with random initial position
	SDL_Rect player = { rand() % (WINDOW_WIDTH - 50), rand() % (WINDOW_HEIGHT - 50), 50, 50 };
	SDL_Rect player2 = { rand() % (WINDOW_WIDTH - 50), rand() % (WINDOW_HEIGHT - 50), 50, 50 };

	// Fruit rects
	SDL_Rect apple = { rand() % (WINDOW_WIDTH - 50), rand() % (WINDOW_HEIGHT - 50),	50,	50 };
	SDL_Rect pear = { rand() % (WINDOW_WIDTH - 50), rand() % (WINDOW_HEIGHT - 50),	50,	50 };

	SDL_Color appleColor = { 255, 0, 0, 255 }; // Red color for apple
	SDL_Color pearColor = { 0, 255, 0, 255 }; // Green color for pear

	bool running = true;

	SDL_Event event;
	while (running) {
		// Handle events
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = false; // Exit the loop if the window is closed
			}
			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
				running = false; // Exit the loop if the Escape key is pressed
			}
			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
				// Reset player position to the center of the window
				player.x = WINDOW_WIDTH / 2 - 25;
				player.y = WINDOW_HEIGHT / 2 - 25;
			}
		}

		// Check for out of bounds for player and player2
		checkForOutOfBounds(player);
		checkForOutOfBounds(player2);

		// Check for collision with fruits
		checkForCollisionWithFruits(player, apple);
		checkForCollisionWithFruits(player2, pear);
		

		// Clear the renderer
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black color
		SDL_RenderClear(renderer);

		// Render the apple
		renderFruits(renderer, apple, appleColor);
		
		// Render the pear
		renderFruits(renderer, pear, pearColor);

		// Render the bigger player first
		if (player.w < player2.w) {
			renderPlayer(renderer, player);
			renderPlayer2(renderer, player2);
		} else {
			renderPlayer2(renderer, player2);
			renderPlayer(renderer, player);
		}

		// Handle player input
		handlePlayerInput(player);
		
		// Handle player2 input
		handlePlayer2Input(player2);
		
		// Present the renderer
		SDL_RenderPresent(renderer);

		// Check for collision between player and apple
		if (SDL_HasIntersection(&player, &apple)) {
			// Reset apple position to a new random location
			apple.x = rand() % (WINDOW_WIDTH - 50);
			apple.y = rand() % (WINDOW_HEIGHT - 50);
			// Increase player size
			player.w += 10;
			player.h += 10;
			// Ensure player does not exceed window bounds
			if (player.x < 0) player.x = 0;
			if (player.y < 0) player.y = 0;
			if (player.x + player.w > WINDOW_WIDTH) player.x = WINDOW_WIDTH - player.w;
			if (player.y + player.h > WINDOW_HEIGHT) player.y = WINDOW_HEIGHT - player.h;
		}

		// Delay to control frame rate
		SDL_Delay(16); // Approximately 60 FPS
	}

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

	return 0;
}
