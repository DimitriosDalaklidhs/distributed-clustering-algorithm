Student Information
Name: Dimitrios Dalaklidis

**Project Description**

This program implements the Distributed Clustering Algorithm (DCA) for a connected undirected graph.
Each node has a unique ID and a unique weight.

The algorithm runs in rounds.
A node can only decide after all of its higher-weight neighbors have decided.

A node becomes either:
Cluster Head (CH)
or
Cluster Member by joining a neighboring CH.

During execution, the program displays:
“wait”, CH(ID), JOIN(ID1, ID2)

After the algorithm completes, the program prints the final cluster configuration.
The program also creates a Graphviz DOT file named graph.dot for visualization.

**Compilation Instructions**

Using **GCC**:

gcc DCA.c -o DCA


On **Windows**:

gcc DCA.c -o DCA.exe

**Execution Instructions**

**Linux:**

./DCA


**Windows:**

DCA.exe

**Input Format**

The program expects the following input from the user:

First line: N M (number of nodes, number of edges)
Next N lines: ID weight
Next M lines: ID1 ID2 (edges)

**Example input:**

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

**Output Description**

The program prints the decision of each node per round.
It prints “wait” when a node cannot decide yet.
It prints CH(ID) when a node becomes a cluster head.
It prints JOIN(ID1, ID2) when a node joins a cluster.

After all nodes decide, the program prints the final clusters.
The program also generates the file graph.dot.

**Graph Visualization (Graphviz)**

To generate a PNG image:

dot -Tpng graph.dot -o graph.png


The produced image shows:
Cluster Heads as double circles
Cluster Members as single circles
Each cluster in a different color
