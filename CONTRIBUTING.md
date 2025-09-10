# Contributing to CLI Games Pack

Thank you for your interest in contributing to CLI Games Pack! This document provides guidelines for contributing to this project.

## 🤝 Ways to Contribute

### 🐛 Bug Reports
- Use the GitHub issue tracker
- Include detailed steps to reproduce
- Mention your operating system and compiler version
- Provide expected vs actual behavior

### 💡 Feature Requests
- Check existing issues first
- Clearly describe the proposed feature
- Explain why it would be useful
- Consider implementation complexity

### 🎮 Adding New Games
- Follow the existing code structure
- Place new game in `games/` directory
- Update `games.h` with function declarations
- Update `main.c` menu system
- Add to `Makefile` sources list

### 📖 Documentation
- Fix typos or unclear instructions
- Add code comments
- Improve README sections
- Update technical documentation

## 🛠️ Development Setup

1. **Fork the repository**
2. **Clone your fork:**
   ```bash
   git clone https://github.com/yourusername/cli-games-pack.git
   cd cli-games-pack
   ```
3. **Create a branch:**
   ```bash
   git checkout -b feature/your-feature-name
   ```
4. **Make your changes**
5. **Test thoroughly**
6. **Commit and push:**
   ```bash
   git commit -m "Add: description of your changes"
   git push origin feature/your-feature-name
   ```
7. **Open a Pull Request**

## 📝 Code Style Guidelines

### General Principles
- Write clean, readable code
- Use consistent indentation (4 spaces)
- Add meaningful comments
- Keep functions focused and small
- Use descriptive variable names

### C-Specific Guidelines
- Follow C99 standard
- Use standard library functions only
- Avoid dynamic memory allocation when possible
- Validate all user inputs
- Handle edge cases gracefully

### Function Naming
```c
// Use descriptive, verb-based names
void play_your_game(void);
void display_game_rules(void);
int get_user_choice(void);
```

### Variable Naming
```c
// Use snake_case for variables
int player_score = 0;
char game_choice[10];
```

### Code Organization
```c
// Include guards in headers
#ifndef GAMES_H
#define GAMES_H

// Function declarations
void play_your_game(void);

#endif // GAMES_H
```

## 🎮 Adding a New Game

### Step-by-Step Guide

1. **Create the game file:**
   ```bash
   touch games/your_game.c
   ```

2. **Include necessary headers:**
   ```c
   #include "games.h"
   ```

3. **Implement the main function:**
   ```c
   void play_your_game(void) {
       // Game logic here
       printf("Welcome to Your Game!\n");
       // ... rest of implementation
   }
   ```

4. **Add to games.h:**
   ```c
   void play_your_game(void);
   ```

5. **Update main.c menu:**
   - Add menu option
   - Add case in switch statement
   - Update choice validation

6. **Update Makefile:**
   ```makefile
   SOURCES = main.c $(SRCDIR)/existing_games.c $(SRCDIR)/your_game.c
   ```

### Game Structure Template
```c
#include "games.h"

void play_your_game(void) {
    // Game initialization
    printf("=== YOUR GAME ===\n");
    
    // Main game loop
    int playing = 1;
    while (playing) {
        // Display game state
        // Get user input
        // Process input
        // Update game state
        // Check win/lose conditions
    }
    
    // Cleanup and scoring
    printf("Thanks for playing!\n");
}
```

## 🧪 Testing Guidelines

### Before Submitting
- [ ] Code compiles without warnings
- [ ] Game runs without crashes
- [ ] All menu options work correctly
- [ ] Input validation handles edge cases
- [ ] Memory leaks checked (if applicable)
- [ ] Cross-platform compatibility considered

### Manual Testing Checklist
- [ ] Test normal gameplay
- [ ] Test invalid inputs
- [ ] Test edge cases (empty input, very long input)
- [ ] Test game exit conditions
- [ ] Test return to main menu

## 📋 Pull Request Guidelines

### PR Title Format
```
Add: Brief description of changes
Fix: Brief description of bug fix
Update: Brief description of updates
Docs: Brief description of documentation changes
```

### PR Description Template
```markdown
## What does this PR do?
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Documentation update
- [ ] Code refactoring

## Testing
- [ ] Compiled successfully
- [ ] Tested manually
- [ ] All existing games still work

## Screenshots (if applicable)
Add screenshots of new features or games
```

## 🎖️ Recognition

Contributors will be recognized in:
- README.md acknowledgments section
- GitHub contributors list
- Release notes (for significant contributions)

## 📞 Getting Help

- Open an issue for questions
- Check existing documentation
- Look at existing game implementations for examples

## 📄 License

By contributing, you agree that your contributions will be licensed under the MIT License.

---

Thank you for contributing to CLI Games Pack! 🎮
