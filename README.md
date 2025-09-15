# 🎮 CLI Games Pack

A collection of 16 classic command-line games written in C, featuring Rock-Paper-Scissors, Guess the Number, Tic Tac Toe, Hangman, Word Scramble, Coin Flip, Blackjack, Bulls & Cows (Mastermind), ASCII Racing Game, and the addictive 2048 puzzle!

I use ASCII and not emojis because I do not wish to someone 's the programm not running smoothly sawing them some weird characters!!!

![C](https://img.shields.io/badge/language-C-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)

## 🎯 Games Included

### 1. 🪨 Rock-Paper-Scissors
- Play against computer opponent
- Best of 3, 5, or 10 rounds
- Win/loss statistics tracking
- Smart computer AI with patterns

### 2. 🔢 Guess the Number
- 4 difficulty levels (Easy to Expert)
- Range from 1-50 to 1-500
- Smart hint system (Higher/Lower)
- Limited or unlimited attempts

### 3. ⭕ Tic Tac Toe
- 2-player local multiplayer
- Clean ASCII board display
- Win detection for rows, columns, diagonals
- Draw game detection

### 4. 🎪 Hangman
- 40+ programming/computer terms
- ASCII hangman drawing
- Wrong guess counter (6 maximum)
- Educational vocabulary focus

### 5. 🔤 Word Scramble
- 40+ technology-related words
- Smart scrambling algorithm
- Hint system (length, first letter, category)
- 3 attempts per word with scoring

### 6. 🪙 Coin Flip
- Single flip and tournament modes
- Statistics tracking (accuracy, streaks)
- Animated coin flip display
- Best streak achievements

### 7. 🃏 Blackjack (21)
- Authentic casino rules
- Betting system with $100 starting chips
- Hit, Stand, Double Down options
- Proper Ace handling (1 or 11)
- Dealer follows standard rules
- 3:2 blackjack payouts

### 8. 🎯 Bulls & Cows (Mastermind)
- 4-digit number guessing logic puzzle
- Bulls = correct digit in correct position
- Cows = correct digit in wrong position
- Smart hint system and strategy tips
- 10 attempt maximum per game
- Personal best score tracking

### 9. 🏎️ ASCII Racing Game
- Top-down racing with falling obstacles
- Real-time keyboard controls (WASD/Arrow keys)
- Increasing speed and difficulty
- Collision detection and scoring
- Cross-platform input handling

### 10. 🧩 2048 Puzzle Game
- Classic 4x4 grid tile sliding puzzle
- WASD controls for tile movement
- Strategic tile merging (2+2=4, 4+4=8, etc.)
- Goal: Reach the 2048 tile
- Score tracking with smart random spawning
- Win/lose condition detection

## 🚀 Quick Start

### Prerequisites
- GCC compiler
- Make (optional but recommended)
- Windows, Linux, or macOS

### Installation

1. **Clone the repository:**
```bash
git clone https://github.com/kstra3/cli-games-pack.git
cd cli-games-pack
```

2. **Compile the games:**
```bash
make
```

3. **Run the games:**
```bash
./games-pack        # Linux/macOS
games-pack.exe      # Windows
```

Or on Windows, simply double-click `play.bat`

### Manual Compilation
If you don't have Make installed:

```bash
gcc -o games-pack main.c games/*.c -std=c99 -Wall
```

## 🎮 How to Play

1. Run the executable
2. Choose a game from the menu (1-10)
3. Follow the on-screen instructions
4. Have fun!

```
+==========================================+
|           CLI GAMES PACK v1.3            |
|         Welcome to Fun Gaming!           |
+==========================================+
|                                          |
|  1. Rock, Paper, Scissors                |
|  2. Guess the Number                     |
|  3. Tic Tac Toe (2 Players)             |
|  4. Hangman                              |
|  5. Word Scramble                        |
|  6. Coin Flip                            |
|  7. Blackjack                            |
|  8. Bulls & Cows (Mastermind)           |
|  9. ASCII Racing Game                    |
|  10. 2048 Puzzle Game                    |
|  11. Exit                                |
|                                          |
+==========================================+
```

## 📁 Project Structure

```
cli-games-pack/
├── main.c                    # Main menu and game launcher
├── games/
│   ├── games.h              # Function declarations and includes
│   ├── rock_paper_scissors.c
│   ├── guess_number.c
│   ├── tic_tac_toe.c
│   ├── hangman.c
│   ├── word_scramble.c
│   ├── coin_flip.c
│   ├── blackjack.c
│   ├── bulls_cows.c
│   ├── ascii_racing.c
│   └── 2048.c               # NEW: 2048 puzzle game
├── Makefile                 # Build automation
├── play.bat                 # Windows launcher script
├── README.md                # This file
├── LICENSE                  # MIT License
└── .gitignore              # Git ignore rules
```

## 🛠️ Technical Details

- **Language:** C (C99 standard)
- **Libraries:** Standard C libraries only (`stdio.h`, `stdlib.h`, `time.h`, `string.h`, `ctype.h`)
- **Compatibility:** Windows, Linux, macOS
- **Memory Management:** No dynamic allocation, stack-based only
- **Input Validation:** Comprehensive error handling for all user inputs
- **Code Style:** Clean, readable, well-documented

## 🎯 Features

- **Modular Design:** Each game is in its own file
- **Cross-Platform:** Works on Windows, Linux, and macOS
- **No Dependencies:** Uses only standard C libraries
- **Educational:** Great for learning C programming
- **Extensible:** Easy to add new games
- **User-Friendly:** Clear menus and instructions


## 🤝 Contributing

Contributions are welcome! Here are some ways you can contribute:

- 🐛 Report bugs
- 💡 Suggest new features
- 🎮 Add new games
- 📖 Improve documentation
- 🎨 Enhance UI/UX

### Development Setup

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/amazing-game`
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🎖️ Acknowledgments

- Inspired by classic arcade and casino games
- Built with passion for retro gaming
- Thanks to all contributors and players!

## 📞 Support

If you encounter any issues or have questions:

 Create a new issue if needed
 Provide detailed information about your problem

## 🌟 Show Your Support

Give a ⭐ if this project helped you or if you enjoyed playing!

---

**Happy Gaming! 🎮**
