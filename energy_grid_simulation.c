#include <SDL2/SDL.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 600
#define GRID_SIZE 12
#define CELL_SIZE (WINDOW_WIDTH / GRID_SIZE)
#define NUM_COLORS 16
#define PI 3.14159265358979323846

typedef struct {
    float energy;
    float frequency;
    int colorIndex;
    float phase;
    float parameterX;
    float parameterY;
    float amplitude;
} Cell;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
Cell grid[GRID_SIZE][GRID_SIZE];
Uint32 colors[NUM_COLORS];
float simulation_time = 0.0f;

// Precomputed sin table
#define SIN_TABLE_SIZE 1024
float sin_table[SIN_TABLE_SIZE];

float distance(int x1, int y1, int x2, int y2) {
    return sqrtf((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

void initializeSDL() {
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Improved Colorful Energy Grid Simulation", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
}

void cleanupSDL() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

float randf() {
    return (float)rand() / RAND_MAX;
}

float fast_sin(float x) {
    x = fmodf(x, 2 * PI);
    if (x < 0) x += 2 * PI;
    return sin_table[(int)(x / (2 * PI) * SIN_TABLE_SIZE) & (SIN_TABLE_SIZE - 1)];
}

void initializeSinTable() {
    for (int i = 0; i < SIN_TABLE_SIZE; i++) {
        sin_table[i] = sinf(i * 2 * PI / SIN_TABLE_SIZE);
    }
}

void updateCell(int x, int y) {
    Cell* cell = &grid[x][y];
    cell->parameterX = cosf(cell->energy * 2 * PI) * 5;
    cell->parameterY = sinf(cell->energy * 2 * PI) * 5;
    cell->frequency = 1 + cell->energy * 3;
    cell->amplitude = 0.5f + cell->energy * 0.5f;
}

void initializeGrid() {
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int y = 0; y < GRID_SIZE; y++) {
            Cell* cell = &grid[x][y];
            cell->energy = randf();
            cell->frequency = randf() * 3 + 1;
            cell->colorIndex = rand() % NUM_COLORS;
            cell->phase = randf() * 2 * PI;
            updateCell(x, y);
        }
    }
}

void drawCanvas() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Set background to black
    SDL_RenderClear(renderer);

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH, WINDOW_HEIGHT);
    Uint32* pixels = NULL;
    int pitch;
    SDL_LockTexture(texture, NULL, (void**)&pixels, &pitch);

    #pragma omp parallel for collapse(2)
    for (int y = 0; y < WINDOW_HEIGHT; y++) {
        for (int x = 0; x < WINDOW_WIDTH; x++) {
            float r = 0, g = 0, b = 0;

            for (int i = 0; i < GRID_SIZE; i++) {
                for (int j = 0; j < GRID_SIZE; j++) {
                    Cell* cell = &grid[i][j];
                    float dx = (float)x / WINDOW_WIDTH * 2 - 1;
                    float dy = (float)y / WINDOW_HEIGHT * 2 - 1;
                    float cellValue = cell->amplitude * fast_sin((dx * cell->parameterX + dy * cell->parameterY) * cell->frequency + cell->phase + simulation_time * cell->frequency);
                    cellValue = (cellValue + 1) / 2;  // Normalize to [0, 1]

                    Uint32 color = colors[cell->colorIndex];
                    r += ((color >> 16) & 0xFF) * cellValue;
                    g += ((color >> 8) & 0xFF) * cellValue;
                    b += (color & 0xFF) * cellValue;
                }
            }

            // Normalize and apply contrast
            float contrast = 1.5f;
            r = fmaxf(0, fminf(255, (r / (GRID_SIZE * GRID_SIZE) - 128) * contrast + 128));
            g = fmaxf(0, fminf(255, (g / (GRID_SIZE * GRID_SIZE) - 128) * contrast + 128));
            b = fmaxf(0, fminf(255, (b / (GRID_SIZE * GRID_SIZE) - 128) * contrast + 128));

            // Apply color boosting
            float max_component = fmaxf(r, fmaxf(g, b));
            if (max_component > 0) {
                float boost_factor = 255.0f / max_component;
                r *= boost_factor;
                g *= boost_factor;
                b *= boost_factor;
            }

            // Apply a minimum brightness to avoid pure black
            float min_brightness = 0.1f;
            r = fmaxf(r, 255 * min_brightness);
            g = fmaxf(g, 255 * min_brightness);
            b = fmaxf(b, 255 * min_brightness);

            pixels[y * WINDOW_WIDTH + x] = ((Uint32)255 << 24) | ((Uint32)r << 16) | ((Uint32)g << 8) | (Uint32)b;
        }
    }

    SDL_UnlockTexture(texture);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_DestroyTexture(texture);

    SDL_RenderPresent(renderer);
}

void changeCell(int clickedX, int clickedY) {
    const float MAX_EFFECT_RADIUS = sqrtf(2) * GRID_SIZE / 4;
    const float ENERGY_INCREASE = 0.8f;
    const float COLOR_CHANGE_INTENSITY = 3.0f;
    const float PHASE_CHANGE_INTENSITY = PI / 2;

    for (int x = 0; x < GRID_SIZE; x++) {
        for (int y = 0; y < GRID_SIZE; y++) {
            float dist = distance(clickedX, clickedY, x, y);
            
            if (dist <= MAX_EFFECT_RADIUS) {
                float effect_strength = 1.0f - (dist / MAX_EFFECT_RADIUS);
                effect_strength = powf(effect_strength, 3);

                Cell* cell = &grid[x][y];

                // Update energy
                float energy_change = ENERGY_INCREASE * effect_strength;
                cell->energy = fmaxf(0.0f, fminf(1.0f, cell->energy + energy_change));

                // Update color
                int color_change = (int)(COLOR_CHANGE_INTENSITY * effect_strength);
                cell->colorIndex = (cell->colorIndex + color_change) % NUM_COLORS;

                // Update phase
                cell->phase += PHASE_CHANGE_INTENSITY * effect_strength;

                // Add some randomness to prevent convergence
                cell->energy += (randf() - 0.5f) * 0.05f * effect_strength;
                cell->phase += (randf() - 0.5f) * 0.05f * effect_strength;

                updateCell(x, y);
            } else {
                // Slight dampening effect on cells outside the radius
                Cell* cell = &grid[x][y];
                cell->energy = fmaxf(0.0f, cell->energy - 0.01f);
                updateCell(x, y);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    srand((unsigned int)time(NULL));
    initializeSDL();
    initializeSinTable();

    SDL_PixelFormat* format = SDL_AllocFormat(SDL_GetWindowPixelFormat(window));

    // Expanded color palette with more vibrant colors
    colors[0] = SDL_MapRGB(format, 255, 0, 0);    // Red
    colors[1] = SDL_MapRGB(format, 255, 128, 0);  // Orange
    colors[2] = SDL_MapRGB(format, 255, 255, 0);  // Yellow
    colors[3] = SDL_MapRGB(format, 128, 255, 0);  // Lime
    colors[4] = SDL_MapRGB(format, 0, 255, 0);    // Green
    colors[5] = SDL_MapRGB(format, 0, 255, 128);  // Spring Green
    colors[6] = SDL_MapRGB(format, 0, 255, 255);  // Cyan
    colors[7] = SDL_MapRGB(format, 0, 128, 255);  // Light Blue
    colors[8] = SDL_MapRGB(format, 0, 0, 255);    // Blue
    colors[9] = SDL_MapRGB(format, 128, 0, 255);  // Purple
    colors[10] = SDL_MapRGB(format, 255, 0, 255); // Magenta
    colors[11] = SDL_MapRGB(format, 255, 0, 128); // Pink
    colors[12] = SDL_MapRGB(format, 255, 128, 128); // Light Red
    colors[13] = SDL_MapRGB(format, 128, 255, 128); // Light Green
    colors[14] = SDL_MapRGB(format, 128, 128, 255); // Light Blue
    colors[15] = SDL_MapRGB(format, 255, 255, 128); // Light Yellow

    SDL_FreeFormat(format);

    initializeGrid();

    SDL_Event event;
    int quit = 0;
    Uint32 lastFrameTime = SDL_GetTicks();
    const Uint32 targetFPS = 60;
    const Uint32 frameInterval = 1000 / targetFPS;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                int cellX = mouseX / CELL_SIZE;
                int cellY = mouseY / CELL_SIZE;
                changeCell(cellX, cellY);
            }
        }

        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastFrameTime >= frameInterval) {
            simulation_time += 0.016f;
            drawCanvas();
            lastFrameTime = currentTime;
        }
    }

    cleanupSDL();
    return 0;
}