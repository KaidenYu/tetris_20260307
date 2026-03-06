#include <conio.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

// Dimensions of the play field
#define FIELD_WIDTH 10
#define FIELD_HEIGHT 20

// Console dimensions and offsets for drawing
#define SCREEN_WIDTH 40
#define SCREEN_HEIGHT 30
#define FIELD_OFFSET_X 10
#define FIELD_OFFSET_Y 2

// Game State
unsigned char field[FIELD_HEIGHT][FIELD_WIDTH] = {0}; // 0 = empty, 1-7 = colors
int score = 0;
bool gameOver = false;

// Tetromino Shapes: 7 pieces, 4 rotations, 4x4 grid (flattened to 16 chars)
// We'll store them as strings representing the 4x4 grid where 'X' is a block.
const char *tetrominoes[7] = {
    // I
    "..X."
    "..X."
    "..X."
    "..X.",

    // J
    "..X."
    "..X."
    ".XX."
    "....",

    // L
    ".X.."
    ".X.."
    ".XX."
    "....",

    // O
    "...."
    ".XX."
    ".XX."
    "....",

    // S
    "...."
    "..XX"
    ".XX."
    "....",

    // T
    "...."
    "..X."
    ".XXX"
    "....",

    // Z
    "...."
    ".XX."
    "..XX"
    "...."};

// Current Piece State
int currentPiece = 0;
int currentRotation = 0; // 0, 1, 2, 3
int currentX = FIELD_WIDTH / 2 - 2;
int currentY = 0;

// Rotate function for 4x4 array coordinates
int Rotate(int px, int py, int r) {
  switch (r % 4) {
  case 0:
    return py * 4 + px; // 0 degrees
  case 1:
    return 12 + py - (px * 4); // 90 degrees
  case 2:
    return 15 - (py * 4) - px; // 180 degrees
  case 3:
    return 3 - py + (px * 4); // 270 degrees
  }
  return 0;
}

// Logic Functions
bool DoesPieceFit(int nTetromino, int rotation, int posX, int posY) {
  for (int px = 0; px < 4; px++) {
    for (int py = 0; py < 4; py++) {
      // Get index into piece
      int pi = Rotate(px, py, rotation);

      // Get index into field
      int fi_x = posX + px;
      int fi_y = posY + py;

      // Only check collisions if this part of the Tetromino is solid
      if (tetrominoes[nTetromino][pi] == 'X') {
        // Out of bounds check
        if (fi_x < 0 || fi_x >= FIELD_WIDTH || fi_y < 0 ||
            fi_y >= FIELD_HEIGHT) {
          return false;
        }
        // Collision with existing piece check
        if (field[fi_y][fi_x] != 0) {
          return false;
        }
      }
    }
  }
  return true;
}

void LockPiece() {
  for (int px = 0; px < 4; px++) {
    for (int py = 0; py < 4; py++) {
      if (tetrominoes[currentPiece][Rotate(px, py, currentRotation)] == 'X') {
        field[currentY + py][currentX + px] = currentPiece + 1;
      }
    }
  }
}

void ClearLines() {
  int linesCleared = 0;
  for (int y = 0; y < FIELD_HEIGHT; y++) {
    bool bLine = true;
    for (int x = 0; x < FIELD_WIDTH; x++) {
      if (field[y][x] == 0) {
        bLine = false;
        break;
      }
    }

    if (bLine) {
      // Clear line and move down
      for (int j = y; j > 0; j--) {
        for (int x = 0; x < FIELD_WIDTH; x++) {
          field[j][x] = field[j - 1][x];
        }
      }
      // Top row empty
      for (int x = 0; x < FIELD_WIDTH; x++) {
        field[0][x] = 0;
      }
      linesCleared++;
    }
  }
  // Update score
  if (linesCleared > 0) {
    score += (1 << linesCleared) * 100;
  }
}

void SpawnPiece() {
  currentX = FIELD_WIDTH / 2 - 2;
  currentY = 0;
  currentRotation = 0;
  currentPiece = rand() % 7;

  if (!DoesPieceFit(currentPiece, currentRotation, currentX, currentY)) {
    gameOver = true;
  }
}

void HideCursor() {
  HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_CURSOR_INFO info;
  info.dwSize = 100;
  info.bVisible = FALSE;
  SetConsoleCursorInfo(consoleHandle, &info);
}

void SetCursorPosition(int x, int y) {
  HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  COORD coord = {(short)x, (short)y};
  SetConsoleCursorPosition(consoleHandle, coord);
}

void SetupConsole() {
  // Attempt to set a fixed size for the console window
  system("mode con cols=40 lines=30");
  HideCursor();
}

int main() {
  SetupConsole();
  srand((unsigned int)time(NULL));

  // Initial Setup
  SpawnPiece();

  // Game Loop Timing
  DWORD lastTick = GetTickCount();
  int dropInterval = 500; // milliseconds

  while (!gameOver) {
    // Timing
    DWORD currentTick = GetTickCount();
    DWORD deltaTime = currentTick - lastTick;

    // Input Handling
    if (_kbhit()) {
      int key = _getch();
      if (key == 224) { // Arrow keys send 224 then a scan code
        key = _getch();
        switch (key) {
        case 75: // Left
          if (DoesPieceFit(currentPiece, currentRotation, currentX - 1,
                           currentY)) {
            currentX--;
          }
          break;
        case 77: // Right
          if (DoesPieceFit(currentPiece, currentRotation, currentX + 1,
                           currentY)) {
            currentX++;
          }
          break;
        case 80: // Down (Soft Drop)
          if (DoesPieceFit(currentPiece, currentRotation, currentX,
                           currentY + 1)) {
            currentY++;
          }
          break;
        case 72: // Up (Rotate)
          if (DoesPieceFit(currentPiece, currentRotation + 1, currentX,
                           currentY)) {
            currentRotation++;
          }
          break;
        }
      } else if (key == 27) { // ESC
        break;
      } else if (key == ' ') { // Spacebar (Hard Drop)
        while (DoesPieceFit(currentPiece, currentRotation, currentX,
                            currentY + 1)) {
          currentY++;
        }
        // Force drop logic to happen immediately
        deltaTime = dropInterval;
      }
    }

    // Game Logic (Gravity)
    if (deltaTime >= (DWORD)dropInterval) {
      lastTick = currentTick;

      if (DoesPieceFit(currentPiece, currentRotation, currentX, currentY + 1)) {
        currentY++;
      } else {
        LockPiece();
        ClearLines();
        SpawnPiece();
      }
    }

    // --- Rendering ---
    // Create an empty screen buffer
    char screen[SCREEN_HEIGHT][SCREEN_WIDTH];
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
      for (int x = 0; x < SCREEN_WIDTH; x++) {
        screen[y][x] = ' ';
      }
    }

    // Draw Field
    for (int y = 0; y < FIELD_HEIGHT; y++) {
      for (int x = 0; x < FIELD_WIDTH; x++) {
        screen[y + FIELD_OFFSET_Y][x + FIELD_OFFSET_X] =
            (field[y][x] == 0) ? '.' : '#';
      }
    }

    // Draw Current Piece
    for (int px = 0; px < 4; px++) {
      for (int py = 0; py < 4; py++) {
        if (tetrominoes[currentPiece][Rotate(px, py, currentRotation)] == 'X') {
          screen[currentY + py + FIELD_OFFSET_Y]
                [currentX + px + FIELD_OFFSET_X] = '@';
        }
      }
    }

    // Draw text and borders (simplified for now)
    char scoreText[32];
    sprintf(scoreText, "SCORE: %d", score);
    for (int i = 0; scoreText[i] != '\0'; i++) {
      if (SCREEN_WIDTH > FIELD_OFFSET_X + FIELD_WIDTH + 5 + i) {
        screen[FIELD_OFFSET_Y][FIELD_OFFSET_X + FIELD_WIDTH + 5 + i] =
            scoreText[i];
      }
    }

    // Output to console
    SetCursorPosition(0, 0);
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
      screen[y][SCREEN_WIDTH - 1] = '\0'; // ensure null termination for puts
      puts(screen[y]);
    }

    Sleep(50); // Small sleep to prevent 100% CPU usage
  }

  // Game Over screen
  system("cls");
  printf("GAME OVER\n");
  printf("Score: %d\n", score);

  // Wait for clear input
  while (_kbhit())
    _getch();
  printf("Press any key to exit...\n");
  _getch();

  return 0;
}
