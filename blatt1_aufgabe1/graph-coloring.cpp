#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include "../cadical/src/ipasir.h"

using namespace std;

void everyNodeGetsAColor(void *solver, int node, int countOfColors)
{   
    // std::cout << "everyNodeGetsAColor" << std::endl;
    // std::cout << "Node " << node << std::endl;
    for (int i = 0; i < countOfColors; i++)
    {
        // std::cout << "Node " << node << ", Color " << i << ", add literal " << node * 100 + i << std::endl;
        ipasir_add(solver, node * 100 + i);
    }

    // std::cout << "Finish Clause" << std::endl;
    ipasir_add(solver, 0);
}

void atMostOneColorPerNode(void *solver, int node, int countOfColors)
{   
    // std::cout << "atMostOneColorPerNode" << std::endl;
    // std::cout << "Node " << node << std::endl;
    for (int i = 0; i < countOfColors - 1; i++)
    {
        for (int j = i + 1; j < countOfColors; j++)
        {
            // std::cout << "Node " << node << ", Color " << -(node * 100 + i) << std::endl;
            // std::cout << "Node " << node << ", Color " <<  -(node * 100 + j) << std::endl;
            ipasir_add(solver, -(node * 100 + i));
            ipasir_add(solver, -(node * 100 + j));
            ipasir_add(solver, 0);
        }
    }
}

void adjacentNodesHaveDifferentColors(void *solver, int firstNode, int secondNode, int countOfColors)
{
    // std::cout << "adjacentNodesHaveDifferentColors" << std::endl;
    // std::cout << "First Node: " << firstNode << std::endl;
    // std::cout << "Second Node: " << secondNode << std::endl;
    for (int i = 0; i < countOfColors; i++)
    {
        // std::cout << "First Node: " << -(firstNode * 100 + i) << std::endl;
        ipasir_add(solver, -(firstNode * 100 + i));

        // std::cout << "Second Node: " << -(secondNode * 100 + i) << std::endl;
        ipasir_add(solver, -(secondNode * 100 + i));

        // std::cout << "Finish Clause" << std::endl;
        ipasir_add(solver, 0);
    }
}

std::string getColor(int number) {
    // Define an array of ANSI escape codes for 20 different colors
    const std::vector<std::string> colors = {
        "\033[30m",   // Black
        "\033[31m",   // Red
        "\033[32m",   // Green
        "\033[33m",   // Yellow
        "\033[34m",   // Blue
        "\033[35m",   // Magenta
        "\033[36m",   // Cyan
        "\033[37m",   // White
        "\033[90m",   // Light Black
        "\033[91m",   // Light Red
        "\033[92m",   // Light Green
        "\033[93m",   // Light Yellow
        "\033[94m",   // Light Blue
        "\033[95m",   // Light Magenta
        "\033[96m",   // Light Cyan
        "\033[97m",   // Light White
        "\033[40m",   // Background Black
        "\033[41m",   // Background Red
        "\033[42m",   // Background Green
        "\033[43m",   // Background Yellow
        "\033[44m",   // Background Blue
    };

    // Calculate the index in the colors array based on the number
    int colorIndex = (number - 1) % colors.size();

    // Return the corresponding color
    return colors[colorIndex];
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Please enter only the path to your dimacs graph file!" << std::endl;
        return 0;
    }

    std::string pathToDimacsGraph = argv[1];
    std::ifstream file(pathToDimacsGraph);
    if (!file.is_open())
    {
        std::cout << "Failed to open the file at the path '" << pathToDimacsGraph << "'!" << std::endl;
        return 1;
    }

    std::cout << "Start parsing the input file ..." << std::endl;

    std::string line;
    int countOfVertices = 0;
    int countOfEdges = 0;
    std::vector<std::vector<int>> adjacencyMatrix;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "p")
        {
            std::string format;
            iss >> format;
            if (format == "edge")
            {
                iss >> countOfVertices >> countOfEdges;
                adjacencyMatrix.assign(countOfVertices, std::vector<int>(countOfEdges, 0));
            }
        }
        else if (token == "e")
        {
            int vertex1, vertex2;
            iss >> vertex1 >> vertex2;
            adjacencyMatrix[vertex1 - 1][vertex2 - 1] = 1; // Assuming 1-based indexing
            adjacencyMatrix[vertex2 - 1][vertex1 - 1] = 1; // Assuming an undirected graph
        }
    }

    file.close();

    std::cout << "File parsed successfully!" << std::endl;
    std::cout << "Start creating the clauses for " << countOfVertices << " Vertices and " << countOfEdges << " Edges!" << std::endl;
    bool satisifable = false;
    int solutionCountOfColors = 0;
    void *solver;
    for (int countOfColors = 2; satisifable == false; countOfColors++)
    {
        std::cout << "Count of Colors: " << countOfColors << std::endl;
        solver = ipasir_init();
        for (int i = 0; i < countOfVertices; i++)
        {
            int variableCodeForNode = i + 1;
            everyNodeGetsAColor(solver, variableCodeForNode, countOfColors);
            atMostOneColorPerNode(solver, variableCodeForNode, countOfColors);

            for (int j = 0; j < countOfVertices; j++)
            {
                if (adjacencyMatrix[i][j] == 1)
                {
                    adjacentNodesHaveDifferentColors(solver, variableCodeForNode, j + 1, countOfColors);
                }
            }
        }

        std::cout << "All clauses created. Calling the solver ..." << std::endl;
        int result = ipasir_solve(solver);
        if (result == 10)
        {
            std::cout << "SAT: Lösung gefunden für " << countOfColors << " Farben!" << std::endl;
            solutionCountOfColors = countOfColors;
            satisifable = true;
        }
        else if (result == 20)
        {
            std::cout << "UNSAT: Keine Lösung gefunden für " << countOfColors << " Farben!" << std::endl;
        }
        else
        {
            std::cout << "CaDiCal-Fehler: " << result << std::endl;
            return 1;
        }
    }

    std::cout << "Now printing the color code for every node" << std::endl;
    for(int i = 1; i <= countOfVertices; i++) {
        int colorCode = 0;
        for(int j = 0; j < solutionCountOfColors; j++) {
            // std::cout << "Node " << i << ", Color " << j << ", Value " << ipasir_val(solver, i * 100 + j) << "." << std::endl;
            int value = ipasir_val(solver, i * 100 + j);
            if(value > 0) {
                colorCode = j;
                break;
            }
        }

        std::cout << getColor(colorCode) << "Node " << i << " has Color " << colorCode + 1 << "!" << "\033[0m " << std::endl;
    }

    ipasir_release(solver);
    return 0;
}