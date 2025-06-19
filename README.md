# RumorFlow (RF): A Social-Inspired Collective Intelligence Metaheuristic

## Motivation

RumorFlow (RF) is a novel population-based metaheuristic that models the social dynamics of rumor propagation. The algorithm operates on a population of solutions structured as a relationship graph, where individuals (solutions) communicate with each other, propagating and refining their "versions of the rumor" (candidate solutions) over time.

Communication is a fundamental aspect of human societies. For millennia, information, along with myths, legends, and rumors, has been transmitted by word of mouth, adapting and evolving with each interaction. This dynamic process of information propagation and modification in a social network serves as inspiration for this new metaheuristic.

The main deficiency that RF seeks to address in other population algorithms is their often simplified interaction model. In a canonical Genetic Algorithm, selection for crossover is global (panmixia), while in PSO the influence of the "global best" creates a star topology. RF proposes a more decentralized and realistic model, where information (good characteristics of a solution) flows locally through a network topology that can also evolve.

## Project Structure

```
.
├── CMakeLists.txt          # Main CMake configuration file
├── main.cpp               # Main program entry point
├── code/                  # Testing functions (CEC17 benchmark)
│   ├── cec17_test_func.c
│   ├── cec17.c
│   └── cec17.h
├── common/               # Common utilities and base classes
│   ├── mh.h             # Base metaheuristic class
│   ├── random.hpp       # Random number generation utilities
│   ├── solution.h       # Solution representation
│   └── util.h           # General utilities
├── inc/                 # Header files
│   ├── population.h     # Population management
│   ├── problem.h        # Problem interface
│   ├── RF.h            # RumorFlow algorithm
│   └── RFGroups.h      # RumorFlow with groups variant
└── src/                # Source files
    ├── population.cpp
    ├── problem.cpp
    ├── RF.cpp
    └── RFGroups.cpp
```

## Building and Running

1. Create a build directory and navigate to it:
```bash
mkdir -p build && cd build
```

2. Configure the project with CMake:
```bash
cmake ..
```

3. Build the project:
```bash
make
```

4. Run the program:
```bash
./main
```

## Requirements

- C++ compiler with C++17 support
- CMake 3.31 or higher

## License

See the [LICENSE](LICENSE) file for details.
