# 🍎 Fruits - Agar.io-like Game

A simple 2D agar.io-like game built with SDL2 where fruits move around, eat smaller fruits to grow, and can shoot projectiles in AI mode.

## 🎮 Game Modes

### PvP Mode (Default)
- **Apple (Red)**: WASD to move
- **Pear (Green)**: Arrow keys to move
- No shooting, pure movement and fruit eating competition

### AI Mode
- **Apple (Red)**: WASD to move, Mouse click to shoot (only in AI mode)
- **AI Fruits**: Pear (Green), Lemon (Yellow), Blueberry (Blue), Orange (Orange)
- Up to 4 AI fruits, each with their own color and fruit
- AI shooting is OFF by default, toggle with **T**
- User can select number of AI fruits with `--ai N` (N=1-4)

## 🚀 Installation & Compilation

### Prerequisites
- **SDL2 development libraries**
- **C++ compiler** (GCC, Clang, or MSVC)

### Linux (Ubuntu/Debian)
```bash
sudo apt-get install libsdl2-dev
g++ fruits.cpp -o fruits -lSDL2
./fruits
```

### Linux (Arch/Artix)
```bash
sudo pacman -S sdl2
g++ fruits.cpp -o fruits -lSDL2
./fruits
```

### macOS
```bash
brew install sdl2
g++ fruits.cpp -o fruits -lSDL2
./fruits
```

### Windows (MinGW)
```bash
# Download SDL2 from https://www.libsdl.org/download-2.0.php
g++ fruits.cpp -o fruits.exe -lSDL2main -lSDL2
./fruits.exe
```

## 🎯 Usage

### PvP Mode (Default)
```bash
./fruits
```
- Apple vs Pear, no shooting

### AI Mode (with 1-4 AI fruits)
```bash
./fruits --ai 4
```
- Apple vs Pear, Lemon, Blueberry, Orange
- Only Apple can shoot (mouse, in AI mode)
- AI shooting is OFF by default, toggle with **T**

## 🎮 Controls

### Apple (Red)
- **Movement**: WASD keys
- **Shooting**: Mouse click (AI mode only)

### Pear (Green), Lemon (Yellow), Blueberry (Blue), Orange (Orange)
- **Movement**: Arrow keys (Pear in PvP), automatic for all AIs in AI mode
- **Shooting**: Only AI fruits can shoot, and only if AI shooting is enabled (T)

### Global Controls
- **T**: Toggle AI shooting (AI mode only)
- **P**: Show projectile counts
- **ESC**: Quit game

## 🍎 Game Mechanics

### Fruit Eating
- Each fruit has its own target fruit (20x20)
- Eating a target fruit increases size by 10x10 and resets projectiles to 3
- Minimum fruit size is 20x20

### Projectiles (AI Mode Only)
- Only Apple can shoot (mouse click)
- AI fruits can shoot if AI shooting is enabled
- Projectiles reduce opponent's size by 10 (min 20x20)
- Projectiles disappear on hit or out of bounds

### Win Condition
- Any fruit that covers the entire screen wins
- Console displays the winner's name

### Transparency Effects
- Fruits become semi-transparent when overlapping other fruits or their target

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

**Enjoy the game!** 🎮🍏🍐🍋🫐🍊
