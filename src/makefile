# Katiztic — build the meadow vibe slice.
#
#   make        build ./katiztic
#   make run    build and run
#   make clean  remove build artifacts
#
# Requires SDL3 and a C compiler.
#   macOS:  brew install sdl3 pkg-config
#   Arch:   sudo pacman -S sdl3 gcc make

CC      := cc

# Prefer pkg-config; it works on both Homebrew and Arch once SDL3 is installed.
# On Apple Silicon, Homebrew lives in /opt/homebrew — make sure pkg-config can
# see it (brew does this automatically if `brew` is on your PATH).
SDL_CFLAGS := $(shell pkg-config --cflags sdl3 2>/dev/null)
SDL_LIBS   := $(shell pkg-config --libs   sdl3 2>/dev/null)

CFLAGS  := -std=c11 -Wall -Wextra -Wpedantic -O2 $(SDL_CFLAGS)
LDFLAGS := $(SDL_LIBS) -lm

SRC     := $(wildcard src/*.c)
OBJ     := $(SRC:.c=.o)
BIN     := katiztic

.PHONY: all run clean

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(BIN)
	./$(BIN)

clean:
	rm -f $(OBJ) $(BIN)