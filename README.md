# sat-solving
Practical Sat Solving Übung an der Hochschule Karlsruhe im WS 24/25

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
cd ../blatt1_aufgabe1
make
```

4. Run the application:
```
./graph-coloring ./examples/<file>
```
