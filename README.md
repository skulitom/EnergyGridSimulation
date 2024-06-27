# Energy Grid Simulation

This project is an interactive, colorful energy grid simulation implemented in C using the SDL2 library. It creates a visually appealing display of energy patterns that users can interact with in real-time.

## Features

- Interactive 12x12 grid of energy cells
- Smooth, colorful animations using SDL2
- Real-time user interaction through mouse clicks
- Efficient rendering using OpenMP parallelization
- Dynamic energy propagation and color changes
- Optimized performance with precomputed sin table

## Requirements

- C compiler (GCC recommended)
- SDL2 library
- OpenMP support (for parallelization)

## Building the Project

1. Ensure you have SDL2 installed on your system. On Ubuntu or Debian-based systems, you can install it using:
   ```
   sudo apt-get install libsdl2-dev
   ```

2. Clone this repository:
   ```
   git clone https://github.com/yourusername/energy-grid-simulation.git
   cd energy-grid-simulation
   ```

3. Compile the project:
   ```
   gcc -o energy_grid_simulation energy_grid_simulation.c -lSDL2 -lm -fopenmp -O3
   ```

## Running the Simulation

After building the project, run the simulation with:

```
./energy_grid_simulation
```

## How to Interact

- Click anywhere on the grid to introduce energy and create ripple effects.
- Watch as the energy propagates and colors change dynamically.
- Close the window to exit the simulation.

## Code Structure

- `initializeSDL()`: Sets up the SDL window and renderer.
- `initializeSinTable()`: Precomputes a sin table for faster calculations.
- `initializeGrid()`: Sets up the initial state of the energy grid.
- `updateCell()`: Updates the properties of a single cell.
- `drawCanvas()`: Renders the current state of the grid.
- `changeCell()`: Handles user interaction and updates the grid accordingly.
- `main()`: The main loop that runs the simulation and handles events.

## Performance Optimization

This simulation uses several optimization techniques:
- OpenMP for parallel processing of pixel calculations.
- Precomputed sin table for faster trigonometric calculations.
- Efficient memory usage with SDL textures.

## Contributing

Contributions to improve the simulation or add new features are welcome. Please feel free to submit pull requests or open issues for any bugs or suggestions.

## License

This project is open-source and available under the MIT License.
