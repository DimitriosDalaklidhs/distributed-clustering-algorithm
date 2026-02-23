# Distributed Clustering Algorithm (DCA)

An implementation of the Distributed Clustering Algorithm for connected undirected graphs, where each node has a unique ID and weight.

---

## How the Algorithm Works

The algorithm runs in rounds. A node can only decide once all of its higher-weight neighbors have decided, at which point it becomes either:

- **Cluster Head (CH)** — if it has the highest weight among its undecided neighbors
- **Cluster Member** — by joining a neighboring CH

During execution the program outputs one of three messages per node per round:
- `wait` — node is still waiting on higher-weight neighbors
- `CH(ID)` — node has become a cluster head
- `JOIN(ID1, ID2)` — node has joined a cluster

---

## Compilation

**Linux / macOS:**
```bash
gcc DCA.c -o DCA
```

**Windows:**
```bash
gcc DCA.c -o DCA.exe
```

---

## Usage

**Linux / macOS:**
```bash
./DCA
```

**Windows:**
```bash
DCA.exe
```

---

## Input Format
```
N M
ID weight       (repeated N times)
ID1 ID2         (repeated M times)
```

**Example:**
```
6 8
A 10
B 15
C 8
D 20
E 12
F 5
A B
A C
B D
B E
C E
C F
D E
E F
```

---

## Output

The program prints each node's decision per round as the algorithm progresses, then prints the final cluster configuration once all nodes have decided.

It also generates a `graph.dot` file for visualization.

---

## Graph Visualization

Requires [Graphviz](https://graphviz.org/). To generate a PNG:
```bash
dot -Tpng graph.dot -o graph.png
```

The output image shows:
- **Cluster Heads** as double circles
- **Cluster Members** as single circles
- Each cluster in a distinct color
