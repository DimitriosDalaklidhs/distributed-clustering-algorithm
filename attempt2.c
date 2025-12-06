#include <stdio.h> /* Include standard input/output library functions (printf, scanf, etc.) */
#include <stdlib.h> /* Include standard library functions (malloc, free, exit, etc.) */
#include <string.h> /* Include string handling functions (strcpy, strcmp, etc.) */
#include <time.h> /* Include time-related functions (time, srand) */
#include <stdbool.h> /* Include support for the bool type (true/false) */

#define MAX_NODES 100 /* Define maximum number of nodes allowed in the graph */
#define MAX_ID_LENGTH 20 /* Define maximum length for node ID strings*/

/*Structure to represent a node in the graph*/

typedef struct {
    char id[MAX_ID_LENGTH];             /* Unique identifier for the node (e.g., "A", "B", ...) */
    int weight;                         /* Unique weight for the node (used to determine priority) */
    int *neighbors;                     /* Pointer to dynamic array of neighbor indices */
    int *neighbor_weights;              /* Pointer to dynamic array of neighbor weights (mirrors neighbors array) */
    int neighbor_count;                 /* Current number of neighbors of this node */
    int role;                           /* Role of the node: 0 = undecided, 1 = cluster head, 2 = cluster member */
    char cluster_head_id[MAX_ID_LENGTH];/* ID of the cluster head if this node is a member */
    int cluster_id;                     /* Numeric cluster ID for visualization (used for coloring) */
} Node; /* Typedef of struct so we can use "Node" directly */

/* Global variables */
Node nodes[MAX_NODES]; /* Array holding all nodes in the graph (up to MAX_NODES) */
int node_count = 0; /* Current number of nodes actually used in 'nodes' array */

/* Color palette for clusters */
const char* cluster_colors[] = { /* Array of C-strings representing colors for clusters in DOT output */
    "red", "blue", "green", "orange", "purple", "brown", "pink", "gray",
    "olive", "navy", "maroon", "teal", "lime", "aqua", "fuchsia", "silver",
    "yellow", "cyan", "magenta", "darkgreen", "darkblue", "darkred"
}; /* End of color array */
const int max_colors = sizeof(cluster_colors) / sizeof(cluster_colors[0]); /* Number of colors available (used with modulo) */

/* Function prototypes */
void initialize_graph(); /* Allocate neighbor arrays and initialize node metadata */
void add_node(char *id, int weight); /* Add a new node with given ID and weight */
void add_edge(int node1, int node2); /* Add an undirected edge between two node indices */
void print_adjacency_list(); /* Print the graph as an adjacency list */
bool all_higher_weight_neighbors_decided(int current_node); /* Check if all higher-weight neighbors have chosen roles */
bool has_cluster_head_neighbor(int current_node, char *cluster_head_id); /* Check if any neighbor is a cluster head */
void execute_dca_algorithm(); /* Run the Distributed Clustering Algorithm */
void print_clusters(); /* Print final cluster assignments */
void assign_cluster_ids(); /* Assign numeric cluster IDs to cluster heads and members */
void generate_dot_file(); /* Generate a Graphviz DOT file for visualization */
int find_node_by_id(char *id); /* Return index of node with given ID, or -1 if not found */
void shuffle_array(int *array, int size); /* Randomly shuffle an array of integers */

/* Initialize the graph structure */
void initialize_graph() {
    printf("=== Initializing Distributed Clustering Algorithm ===\n\n"); /* Inform user that initialization has started */

    /* Initialize all nodes */
    for (int i = 0; i < MAX_NODES; i++) { /* Loop through maximum possible nodes */
        nodes[i].neighbors = malloc(MAX_NODES * sizeof(int)); /* Allocate space for neighbor indices */
        nodes[i].neighbor_weights = malloc(MAX_NODES * sizeof(int)); /* Allocate space for neighbor weights */
        if (nodes[i].neighbors == NULL || nodes[i].neighbor_weights == NULL) { /* Check if memory allocation failed */
            fprintf(stderr, "Error: Memory allocation failed for node %d\n", i); /* Print error message */
            exit(1); /* Terminate program due to critical failure */
        }
        nodes[i].neighbor_count = 0; /* Initially, each node has no neighbors */
        nodes[i].role = 0; /* Mark node as undecided (no role yet) */
        nodes[i].cluster_id = -1; /* Set cluster ID to -1 to indicate no cluster assigned yet */
        strcpy(nodes[i].cluster_head_id, ""); /* Initialize cluster head ID as an empty string */
    }
}

/* Add a new node to the graph */
void add_node(char *id, int weight) {
    if (node_count >= MAX_NODES) { /* Ensure we do not exceed the maximum allowed number of nodes */
        fprintf(stderr, "Error: Cannot add more nodes. Maximum limit (%d) reached.\n", MAX_NODES); /* Error message */
        exit(1); /* Terminate to avoid buffer overflow */
    }
    
    strcpy(nodes[node_count].id, id);      /* Copy the given ID string into the node's id field */
    nodes[node_count].weight = weight;     /* Set the node's weight to the provided value */
    nodes[node_count].neighbor_count = 0;  /* Start with zero neighbors for this new node */
    nodes[node_count].role = 0;            /* Mark role as undecided initially */
    nodes[node_count].cluster_id = -1;     /* No cluster assigned yet */
    node_count++;                          /* Increase global count of nodes after adding this one */
}

/* Add an edge between two nodes (bidirectional) */
void add_edge(int node1_idx, int node2_idx) {
    /* Check bounds to prevent memory overflow */
    if (nodes[node1_idx].neighbor_count >= MAX_NODES || 
        nodes[node2_idx].neighbor_count >= MAX_NODES) { /* If either node already has too many neighbors */
        fprintf(stderr, "Error: Too many neighbors for node. Maximum limit (%d) reached.\n", MAX_NODES); /* Error message */
        exit(1); /* Terminate due to potential out-of-bounds access */
    }
    
    /* Add node2 as neighbor of node1 */
    nodes[node1_idx].neighbors[nodes[node1_idx].neighbor_count] = node2_idx; /* Store index of node2 in node1's neighbor list */
    nodes[node1_idx].neighbor_weights[nodes[node1_idx].neighbor_count] = nodes[node2_idx].weight; /* Store node2's weight alongside */
    nodes[node1_idx].neighbor_count++; /* Increment neighbor count for node1 */

    /* Add node1 as neighbor of node2 (bidirectional edge) */
    nodes[node2_idx].neighbors[nodes[node2_idx].neighbor_count] = node1_idx; /* Store index of node1 in node2's neighbor list */
    nodes[node2_idx].neighbor_weights[nodes[node2_idx].neighbor_count] = nodes[node1_idx].weight; /* Store node1's weight */
    nodes[node2_idx].neighbor_count++; /* Increment neighbor count for node2 */
}

/* Print adjacency list representation of the graph */
void print_adjacency_list() {
    printf("=== Graph Adjacency List ===\n"); /* Header for adjacency list output */
    for (int i = 0; i < node_count; i++) { /* Loop over all existing nodes */
        printf("Node %s (weight: %d): ", nodes[i].id, nodes[i].weight); /* Print node ID and its weight */
        for (int j = 0; j < nodes[i].neighbor_count; j++) { /* Loop over the node's neighbors */
            int neighbor_idx = nodes[i].neighbors[j]; /* Get index of neighbor j */
            printf("%s(w:%d) ", nodes[neighbor_idx].id, nodes[neighbor_idx].weight); /* Print neighbor ID and its weight */
        }
        printf("\n"); /* New line after listing all neighbors of a node */
    }
    printf("\n"); /* Extra blank line for readability */
}

/* Check if all higher-weight neighbors of current node have decided their role */
bool all_higher_weight_neighbors_decided(int current_node) { 
    int current_weight = nodes[current_node].weight; /* Get the weight of the current node */

    for (int i = 0; i < nodes[current_node].neighbor_count; i++) { /* Loop over all neighbors of the current node */
        int neighbor_idx = nodes[current_node].neighbors[i]; /* Index of neighbor i */

        if (nodes[neighbor_idx].weight > current_weight && /* If neighbor has strictly higher weight */
            nodes[neighbor_idx].role == 0) { /* And neighbor's role is still undecided */
            return false; /* Then not all higher-weight neighbors have decided yet */
        }
    }
    return true; /* If we reach here, all higher-weight neighbors have decided their roles */
}

/* Check if current node has a cluster head neighbor and return its ID */
bool has_cluster_head_neighbor(int current_node, char *cluster_head_id) {
    /* Check all neighbors of current node */
    for (int i = 0; i < nodes[current_node].neighbor_count; i++) { /* Loop over neighbors */
        int neighbor_idx = nodes[current_node].neighbors[i]; /* Get neighbor index */

        /* If neighbor is a cluster head */
        if (nodes[neighbor_idx].role == 1) { /* Check if neighbor's role is cluster head */
            strcpy(cluster_head_id, nodes[neighbor_idx].id); /* Copy neighbor's ID into cluster_head_id buffer */
            return true; /* Return true indicating a cluster head neighbor was found */
        }
    }
    return false; /* Return false if no cluster head neighbors were found */
}

/* Find node index by its ID */
int find_node_by_id(char *id) {
    for (int i = 0; i < node_count; i++) { /* Loop through all nodes */
        if (strcmp(nodes[i].id, id) == 0) { /* Compare stored ID with target ID */
            return i; /* Return index if IDs match */
        }
    }
    return -1; /* Return -1 if no node with this ID was found */
}

/* Shuffle array for random node processing order */
void shuffle_array(int *array, int size) {
    for (int i = size - 1; i > 0; i--) { /* Loop from end of array down to 1 */
        int j = rand() % (i + 1); /* Pick a random index between 0 and i inclusive */

        /* Swap elements at positions i and j */
        int temp = array[i]; /* Temporarily store value at position i */
        array[i] = array[j]; /* Put value from j into i */
        array[j] = temp; /* Put stored value into position j */
    }
}

/* Main DCA algorithm execution */
void execute_dca_algorithm() {
    printf("=== Executing Distributed Clustering Algorithm ===\n"); /* Announce start of algorithm */

    int round = 1; /* Initialize round counter to 1 */
    bool algorithm_complete = false; /* Flag to indicate if the algorithm has finished */

    /* Continue until all nodes have decided their roles */
    while (!algorithm_complete) { /* Loop until algorithm_complete becomes true */
        printf("\n--- Round %d ---\n", round); /* Print round number */

        /* Create array of node indices for random processing */
        int node_order[MAX_NODES]; /* Temporary array for holding indices 0..node_count-1 */
        for (int i = 0; i < node_count; i++) { /* Fill in initial order */
            node_order[i] = i; /* Assign each position i the node index i */
        }

        /* Shuffle the array to process nodes in random order */
        shuffle_array(node_order, node_count); /* Randomize processing order of nodes */

        /* Process each node in random order */
        for (int i = 0; i < node_count; i++) { /* Loop through shuffled indices */
            int current_node = node_order[i]; /* Get current node index from shuffled array */

            /* Skip if node has already decided its role */
            if (nodes[current_node].role != 0) { /* If role is not 0 (undecided) */
                continue; /* Skip to next node since this one already has a role */
            }

            /* Check if all higher-weight neighbors have decided */
            if (!all_higher_weight_neighbors_decided(current_node)) { /* If some higher-weight neighbor still undecided */
                printf("%s - wait\n", nodes[current_node].id); /* Print message that this node must wait */
                continue; /* Skip making decision in this round for this node */
            }

            /* Check if node has a cluster head neighbor */
            char cluster_head_id[MAX_ID_LENGTH]; /* Buffer to store ID of cluster head neighbor */
            if (has_cluster_head_neighbor(current_node, cluster_head_id)) { /* If at least one neighbor is a cluster head */
                /* Join the cluster of the cluster head neighbor */
                nodes[current_node].role = 2; /* Set role as cluster member */
                strcpy(nodes[current_node].cluster_head_id, cluster_head_id); /* Store the cluster head ID */
                printf("JOIN(%s, %s)\n", nodes[current_node].id, cluster_head_id); /* Print join operation */
            } else { /* If no neighbor is a cluster head */
                /* Become a cluster head */
                nodes[current_node].role = 1; /* Set role as cluster head */
                strcpy(nodes[current_node].cluster_head_id, nodes[current_node].id); /* Node's cluster head is itself */
                printf("CH(%s)\n", nodes[current_node].id); /* Print that this node became a cluster head */
            }
        }

        /* Check if algorithm is complete (all nodes have decided) */
        algorithm_complete = true; /* Assume complete initially */
        for (int i = 0; i < node_count; i++) { /* Check every node */
            if (nodes[i].role == 0) { /* If any node is still undecided */
                algorithm_complete = false; /* Mark that we are not done yet */
                break; /* Break out of check loop */
            }
        }

        round++; /* Increment round counter for next iteration */
    }

    printf("\n=== Algorithm Completed ===\n"); /* Print completion message */
}

/* Assign cluster IDs for visualization */
void assign_cluster_ids() {
    int cluster_counter = 0; /* Counter to assign incremental cluster IDs */
    
    /* First pass: assign cluster IDs to cluster heads */
    for (int i = 0; i < node_count; i++) { /* Loop over all nodes */
        if (nodes[i].role == 1) { /* Cluster head role */
            nodes[i].cluster_id = cluster_counter++; /* Assign current cluster_counter then increment it */
        }
    }
    
    /* Second pass: assign cluster IDs to cluster members */
    for (int i = 0; i < node_count; i++) { /* Loop over all nodes again */
        if (nodes[i].role == 2) { /* If node is a cluster member */
            /* Find the cluster head and get its cluster ID */
            for (int j = 0; j < node_count; j++) { /* Search for matching cluster head */
                if (nodes[j].role == 1 &&  /* Node j must be a cluster head */
                    strcmp(nodes[j].id, nodes[i].cluster_head_id) == 0) { /* And its ID must match the member's stored cluster_head_id */
                    nodes[i].cluster_id = nodes[j].cluster_id; /* Assign same cluster ID to this member */
                    break; /* Stop searching once found */
                }
            }
        }
    }
}

/* Generate Graphviz DOT file for visualization */
void generate_dot_file() {
    FILE *dot_file = fopen("graph.dot", "w"); /* Open (or create) file "graph.dot" in write mode */
    if (dot_file == NULL) { /* Check that the file was opened successfully */
        fprintf(stderr, "Error: Could not create graph.dot file\n"); /* Print error if not */
        return; /* Exit the function early */
    }
    
    /* Assign cluster IDs first */
    assign_cluster_ids(); /* Ensure each node has a cluster_id for coloring */
    
    fprintf(dot_file, "graph DCA_Clusters {\n"); /* Start of DOT graph declaration */
    fprintf(dot_file, "    rankdir=LR;\n"); /* Set graph layout direction from left to right */
    fprintf(dot_file, "    node [style=filled, fontsize=12];\n"); /* Default node style: filled nodes, font size 12 */
    fprintf(dot_file, "    edge [color=gray];\n\n"); /* Default edge style: gray color */
    
    /* Add nodes with colors based on clusters */
    for (int i = 0; i < node_count; i++) { /* Loop through all nodes */
        const char* color = cluster_colors[nodes[i].cluster_id % max_colors]; /* Choose color based on cluster_id modulo color palette size */
        const char* shape = (nodes[i].role == 1) ? "doublecircle" : "circle"; /* Cluster heads are drawn as doublecircle, members as circle */
        
        fprintf(dot_file, "    %s [label=\"%s\\n(w:%d)\", color=%s, shape=%s];\n", 
                nodes[i].id, nodes[i].id, nodes[i].weight, color, shape); /* Print node definition with label, color, and shape */
    }
    
    fprintf(dot_file, "\n"); /* Blank line before edges */
    
    /* Add edges (avoid duplicates by only adding when i < j) */
    for (int i = 0; i < node_count; i++) { /* Loop over nodes */
        for (int j = 0; j < nodes[i].neighbor_count; j++) { /* Loop over neighbors of node i */
            int neighbor_idx = nodes[i].neighbors[j]; /* Get neighbor index */
            if (i < neighbor_idx) { /* Only emit edge if i < neighbor_idx to avoid duplicates in an undirected graph */
                fprintf(dot_file, "    %s -- %s;\n", 
                        nodes[i].id, nodes[neighbor_idx].id); /* Print undirected edge between i and its neighbor */
            }
        }
    }
    
    fprintf(dot_file, "}\n"); /* Close DOT graph */
    fclose(dot_file); /* Close the file handle */
    
    printf("\n=== Graph Visualization ===\n"); /* Inform user about DOT output */
    printf("DOT file 'graph.dot' generated successfully!\n"); /* Success message */
    printf("To generate PNG image, run: dot -Tpng graph.dot -o graph.png\n"); /* Instruction for converting DOT to PNG */
    printf("Legend:\n"); /* Start of legend description */
    printf("  - Double circles: Cluster Heads\n"); /* Explain doublecircle shape */
    printf("  - Single circles: Cluster Members\n"); /* Explain circle shape */
    printf("  - Colors represent different clusters\n\n"); /* Explain color usage */
}

/* Print final cluster configuration */
void print_clusters() {
    printf("\n=== Final Cluster Configuration ===\n"); /* Header for cluster configuration output */
    int cluster_num = 1; /* Start cluster numbering from 1 */

    /* Process each cluster head */
    for (int i = 0; i < node_count; i++) { /* Loop through all nodes */
        if (nodes[i].role == 1) { /* If node is a cluster head */
            printf("Cluster %d: CH(%s), members(", cluster_num, nodes[i].id); /* Print cluster head label */
            bool first_member = true; /* Flag to handle commas between member names */

            /* Find all members of this cluster */
            for (int j = 0; j < node_count; j++) { /* Check all nodes to see which ones belong to this cluster head */
                if (nodes[j].role == 2 && strcmp(nodes[j].cluster_head_id, nodes[i].id) == 0) { /* Node j is member of this cluster head */
                    if (!first_member) { /* If this is not the first member printed */
                        printf(", "); /* Print comma separator */
                    }
                    printf("%s", nodes[j].id); /* Print member node ID */
                    first_member = false; /* After first member, set flag to false */
                }
            }
            printf(")\n"); /* Close member list and move to next line */
            cluster_num++; /* Increment cluster number for next cluster head */
        }
    }
}

/* Main function */
int main() {
    /* Seed random number generator */
    srand(time(NULL)); /* Initialize random seed based on current time */

    /* Initialize graph structure */
    initialize_graph(); /* Allocate memory for nodes' neighbor arrays and reset fields */

    int N, M; /* Variables to store number of nodes (N) and edges (M) */
    printf("Enter number of nodes and edges (N M): "); /* Prompt user for N and M */
    if (scanf("%d %d", &N, &M) != 2) { /* Read N and M from standard input and check success */
        fprintf(stderr, "Error: failed to read N and M.\n"); /* Print error if read fails */
        return 1; /* Exit program with error code 1 */
    }
    if (N > MAX_NODES) { /* Ensure N does not exceed maximum allowed nodes */
        fprintf(stderr, "Error: N exceeds MAX_NODES (%d).\n", MAX_NODES); /* Print error if N is too large */
        return 1; /* Exit program with error code */
    }

    /* Read nodes: ID and weight */
    printf("Enter each node as: ID weight\n"); /* Instructions for entering node data */
    for (int i = 0; i < N; i++) { /* Loop N times to read all nodes */
        char id[MAX_ID_LENGTH]; /* Temporary buffer for node ID */
        int weight; /* Temporary variable for node weight */
        if (scanf("%s %d", id, &weight) != 2) { /* Read ID and weight and validate input */
            fprintf(stderr, "Error: failed to read node %d.\n", i); /* Print error message for failed read */
            return 1; /* Exit program */
        }
        add_node(id, weight); /* Add the node to the graph */
    }

    /* Read edges: pairs of IDs */
    printf("Enter each edge as: ID1 ID2\n"); /* Instructions for entering edges */
    for (int k = 0; k < M; k++) { /* Loop M times to read all edges */
        char id1[MAX_ID_LENGTH], id2[MAX_ID_LENGTH]; /* Temporary buffers for the two node IDs per edge */
        if (scanf("%s %s", id1, id2) != 2) { /* Read two strings representing node IDs */
            fprintf(stderr, "Error: failed to read edge %d.\n", k); /* Print error if read fails */
            return 1; /* Exit program */
        }

        int u = find_node_by_id(id1); /* Find index of first node */
        int v = find_node_by_id(id2); /* Find index of second node */
        if (u == -1 || v == -1) { /* If either ID is unknown */
            fprintf(stderr, "Error: unknown node id in edge %s %s.\n", id1, id2); /* Print error message */
            return 1; /* Exit program */
        }

        add_edge(u, v); /* Add undirected edge between u and v */
    }

    /* Print adjacency list */
    print_adjacency_list(); /* Show the graph structure as adjacency list */

    /* Execute DCA algorithm */
    execute_dca_algorithm(); /* Run the distributed clustering logic */

    /* Print final clusters */
    print_clusters(); /* Display which nodes belong to which clusters */

    /* Generate visualization */
    generate_dot_file(); /* Create Graphviz DOT file for visual inspection */

    /* Free allocated memory */
    for (int i = 0; i < MAX_NODES; i++) { /* Loop over all allocated nodes */
        free(nodes[i].neighbors); /* Free neighbor indices array */
        free(nodes[i].neighbor_weights); /* Free neighbor weights array */
    }
    
    return 0; /* Return 0 to indicate successful program execution */
}

