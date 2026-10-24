#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

// Structure to store each road
struct Edge {
    int source;
    int destination;
    int cost;
};

// Find the parent of a vertex
int findParent(vector<int>& parent, int vertex) {
    if (parent[vertex] == vertex)
        return vertex;

    return parent[vertex] =
           findParent(parent, parent[vertex]);
}

// Join two sets
bool joinSets(vector<int>& parent,
              vector<int>& rank,
              int u, int v) {
    int parentU = findParent(parent, u);
    int parentV = findParent(parent, v);

    // If both have the same parent, a cycle would form
    if (parentU == parentV)
        return false;

    if (rank[parentU] < rank[parentV]) {
        parent[parentU] = parentV;
    }
    else if (rank[parentU] > rank[parentV]) {
        parent[parentV] = parentU;
    }
    else {
        parent[parentV] = parentU;
        rank[parentU]++;
    }

    return true;
}

int main() {
    int vertices, edges;

    cout << "MINIMUM SPANNING TREE COST CALCULATOR\n";

    cout << "Enter number of vertices: ";
    cin >> vertices;

    cout << "Enter number of edges: ";
    cin >> edges;

    if (vertices <= 0 || edges < 0) {
        cout << "Invalid input.\n";
        return 0;
    }

    vector<Edge> graph(edges);

    cout << "\nEnter each edge as: source destination cost\n";
    cout << "Use vertex numbers from 0 to "
         << vertices - 1 << ".\n";

    for (int i = 0; i < edges; i++) {
        cin >> graph[i].source
            >> graph[i].destination
            >> graph[i].cost;

        if (graph[i].source < 0 ||
            graph[i].source >= vertices ||
            graph[i].destination < 0 ||
            graph[i].destination >= vertices) {
            cout << "Invalid vertex number.\n";
            return 0;
        }
    }

    // Sort edges by cost, from lowest to highest
    sort(graph.begin(), graph.end(),
         [](const Edge& a, const Edge& b) {
             return a.cost < b.cost;
         });

    vector<int> parent(vertices);
    vector<int> rank(vertices, 0);

    for (int i = 0; i < vertices; i++) {
        parent[i] = i;
    }

    int totalCost = 0;
    int selectedEdges = 0;

    cout << "\nSelected edges:\n";

    // Apply Kruskal's algorithm
    for (int i = 0; i < edges; i++) {
        int u = graph[i].source;
        int v = graph[i].destination;
        int cost = graph[i].cost;

        if (joinSets(parent, rank, u, v)) {
            cout << u << " - " << v
                 << " : " << cost << "\n";

            totalCost += cost;
            selectedEdges++;

            if (selectedEdges == vertices - 1)
                break;
        }
    }

    if (selectedEdges != vertices - 1) {
        cout << "\nThe graph is disconnected.\n";
        cout << "A spanning tree cannot be formed.\n";
    }
    else {
        cout << "\nMinimum Spanning Tree Cost = "
             << totalCost << "\n";
    }

    return 0;
}
