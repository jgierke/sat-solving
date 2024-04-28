# sat-solving

Exercises of lecture Practical SAT Solving at Hochschule Karlsruhe in Summer 2024

## Requirements

* GCC
* Make

## Exercise 1: Graph coloring

1. clone the repository including its submodule:
```
git clone --recurse-submodules https://github.com/jgierke/sat-solving.git
cd sat-solving
```

2. Build the cadical library:
```
cd cadical
./configure && make
```

3. Build the graph coloring application:
```
cd ../exercise1
make
```

4. Run the application:
```
./graph-coloring ./examples/<file>
```
