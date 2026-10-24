Smart MST Visualizer & Cost Analyzer

Project Overview
Smart MST Visualizer & Cost Analyzer is a C++ application that demonstrates the Minimum Spanning Tree (MST) of a weighted graph using Kruskal's Algorithm. It helps users understand how a minimum-cost spanning tree is constructed by selecting edges with minimum weights while avoiding cycles.

Objectives
Understand and implement Kruskal's Algorithm.
Find the Minimum Spanning Tree of a weighted graph.
Calculate the minimum total spanning cost.
Visualize graph vertices and edges through a graphical interface.
Demonstrate the use of Disjoint Set Union (DSU) for cycle detection.

Features
Graphical user interface.
Weighted graph representation.
Minimum Spanning Tree calculation.
Edge selection based on weight.
Cycle detection using Disjoint Set Union.
Total MST cost calculation.

Technologies Used
Programming Language: C++
Algorithm: Kruskal's Algorithm
Data Structure: Disjoint Set Union (DSU)
Graphics: Windows API / GDI

Algorithm
Sort all graph edges in ascending order of weight.
Select the edge with the smallest weight.
Check whether adding the edge creates a cycle using DSU.
If it does not create a cycle, include it in the MST.
Repeat until the MST contains (V-1) edges, where (V) is the number of vertices.
Calculate the total cost of the selected edges.

Academic Project
Project Title: Smart MST Visualizer & Cost Analyzer
Subject: Design and Analysis of Algorithms (DAA)
Department: Information Technology
