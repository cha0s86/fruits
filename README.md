# 🍎 Fruits - Agar.io-like Game

A simple 2D agar.io-like game built with SDL2 where players move around, eat fruits to grow, and can shoot projectiles in AI mode.

## 🎮 Game Modes

### PvP Mode (Default)
- **Player 1 (Red)**: WASD to move
- **Player 2 (Green)**: Arrow keys to move
- Pure movement and fruit eating competition - no shooting!

### AI Mode
- **Player 1 (Red)**: WASD to move, Mouse click to shoot
- **AI (Green)**: Automatic movement + shooting (toggleable)
- Full combat mode with projectiles

## 🚀 Installation & Compilation

### Prerequisites
- **SDL2 development libraries**
- **C++ compiler** (GCC, Clang, or MSVC)

### Linux (Ubuntu/Debian)
```bash
# Install SDL2
sudo apt-get install libsdl2-dev

# Compile the game
g++ fruits.cpp -o fruits -lSDL2

# Run the game
./fruits
```

### Linux (Arch/Artix)
```bash
# Install SDL2
sudo pacman -S sdl2

# Compile the game
g++ fruits.cpp -o fruits -lSDL2

# Run the game
./fruits
```

### macOS
```bash
# Install SDL2 (using Homebrew)
brew install sdl2

# Compile the game
g++ fruits.cpp -o fruits -lSDL2

# Run the game
./fruits
```

### Windows (MinGW)
```bash
# Install SDL2 development libraries
# Download from: https://www.libsdl.org/download-2.0.php

# Compile the game
g++ fruits.cpp -o fruits.exe -lSDL2main -lSDL2

# Run the game
./fruits.exe
```

## 🎯 Usage

### PvP Mode (Default)
```bash
./fruits
```
- Two human players compete
- Move around eating fruits to grow
- No shooting - pure movement competition

### AI Mode
```bash
./fruits --ai
```
- Player vs AI combat
- Player 1 can shoot with mouse
- AI shoots automatically (toggleable)

## 🎮 Controls

### Player 1 (Red)
- **Movement**: WASD keys
- **Shooting**: Mouse click (AI mode only)

### Player 2 (Green) / AI
- **Movement**: Arrow keys (PvP) / Automatic (AI mode)
- **Shooting**: Automatic (AI mode only)

### Global Controls
- **T**: Toggle AI shooting (AI mode only)
- **P**: Show projectile counts
- **ESC**: Quit game

## 🍎 Game Mechanics

### Fruit Eating
- Eat red apples (Player 1) and green pears (Player 2/AI)
- Grow larger with each fruit consumed
- Reset projectile count to 3 when eating

### Projectiles (AI Mode Only)
- Limited to 3 projectiles per player
- Reset to 3 when eating fruits
- Hit opponents to reduce their size
- Projectiles disappear when hitting or going out of bounds

### Win Condition
- Cover the entire screen to win
- Console displays winner message

### Transparency Effects
- Players become semi-transparent when overlapping fruits or each other
- Visual feedback for interactions

## 🏆 Victory Conditions

- **Size-based**: Grow large enough to cover the entire screen
- **Console feedback**: Winner announcement displayed
- **Game ends**: Automatic termination after win

## 🔧 Technical Details

- **Engine**: SDL2 for graphics, input, and window management
- **Language**: C++
- **Architecture**: Single-threaded game loop
- **Rendering**: 60 FPS with alpha blending
- **Collision**: SDL_HasIntersection for precise detection

## 🐛 Troubleshooting

### Compilation Issues
```bash
# If SDL2 not found, ensure development libraries are installed
sudo apt-get install libsdl2-dev  # Ubuntu/Debian
sudo pacman -S sdl2               # Arch/Artix
brew install sdl2                 # macOS
```

### Runtime Issues
- Ensure SDL2 runtime libraries are installed
- Check that the executable has proper permissions
- Verify SDL2 is properly linked

## 📝 License

This project is open source. Feel free to modify and distribute!

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

---

**Enjoy the game!** 🎮🍎
