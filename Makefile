# CLI Games Pack Makefile
# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
TARGET = cli-games
SRCDIR = games
SOURCES = main.c $(SRCDIR)/rock_paper_scissors.c $(SRCDIR)/guess_number.c $(SRCDIR)/tic_tac_toe.c $(SRCDIR)/hangman.c $(SRCDIR)/word_scramble.c $(SRCDIR)/coin_flip.c $(SRCDIR)/blackjack.c $(SRCDIR)/bulls_and_cows.c $(SRCDIR)/ascii_racing.c
OBJECTS = $(SOURCES:.c=.o)

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJECTS)
	@echo "🔗 Linking $(TARGET)..."
	$(CC) $(OBJECTS) -o $(TARGET)
	@echo "✅ Build complete! Executable: $(TARGET)"

# Compile source files
%.o: %.c
	@echo "🔨 Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	@echo "🧹 Cleaning build files..."
	rm -f $(OBJECTS) $(TARGET)
	@echo "✅ Clean complete!"

# Install (copy to system directory - Unix/Linux/macOS)
install: $(TARGET)
	@echo "📦 Installing $(TARGET)..."
	sudo cp $(TARGET) /usr/local/bin/
	@echo "✅ Installation complete! You can now run '$(TARGET)' from anywhere."

# Uninstall (remove from system directory)
uninstall:
	@echo "🗑️  Uninstalling $(TARGET)..."
	sudo rm -f /usr/local/bin/$(TARGET)
	@echo "✅ Uninstall complete!"

# Debug build with debugging symbols
debug: CFLAGS += -g -DDEBUG
debug: clean $(TARGET)
	@echo "🐛 Debug build complete!"

# Run the program
run: $(TARGET)
	@echo "🎮 Starting CLI Games Pack..."
	./$(TARGET)

# Create release build (optimized)
release: CFLAGS += -DNDEBUG -s
release: clean $(TARGET)
	@echo "🚀 Release build complete!"

# Help target
help:
	@echo "CLI Games Pack - Available make targets:"
	@echo "  all      - Build the executable (default)"
	@echo "  clean    - Remove build files"
	@echo "  debug    - Build with debugging symbols"
	@echo "  release  - Build optimized release version"
	@echo "  run      - Build and run the program"
	@echo "  install  - Install to /usr/local/bin (Unix/Linux/macOS)"
	@echo "  uninstall- Remove from /usr/local/bin"
	@echo "  help     - Show this help message"

# Declare phony targets
.PHONY: all clean install uninstall debug release run help

# Dependencies
main.o: main.c $(SRCDIR)/games.h
$(SRCDIR)/rock_paper_scissors.o: $(SRCDIR)/rock_paper_scissors.c $(SRCDIR)/games.h
$(SRCDIR)/guess_number.o: $(SRCDIR)/guess_number.c $(SRCDIR)/games.h
$(SRCDIR)/tic_tac_toe.o: $(SRCDIR)/tic_tac_toe.c $(SRCDIR)/games.h
$(SRCDIR)/hangman.o: $(SRCDIR)/hangman.c $(SRCDIR)/games.h
$(SRCDIR)/word_scramble.o: $(SRCDIR)/word_scramble.c $(SRCDIR)/games.h
$(SRCDIR)/coin_flip.o: $(SRCDIR)/coin_flip.c $(SRCDIR)/games.h
