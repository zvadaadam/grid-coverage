//
// Created by Adam Zvada on 2019-04-30.
//

#ifndef COVERAGE_GRID_H
#define COVERAGE_GRID_H

#include <istream>

#include "block.h"
#include "coverage_problem.h"

using namespace std;

#define BLOCKED -1
#define EMPTY 0
#define TYPE_1 1
#define TYPE_2 2
#define HORIZONTAL 3
#define VERTICAL 4

class Grid {
public:
    Grid(CoverageProblem * problem);
    Grid(Grid * grid, CoverageProblem * problem);
    ~Grid();

    bool addBlockIfPossible(Block * block);
    bool undoBlock(Block * block);
    vector<Block*> generatePossibleBlocks(Point * cord);

    int getCost() { return this->cost; }
    int getCostWithoutPenalty(Point * cord);
    int getGridValue(int i, int j) { return this->grid[i][j]; }
    void updateGridValue(int i, int j, int newVal) { this->grid[i][j] = newVal; }
    int upperBoundCost(Point * cord);
    int lowerBoundCost();

    void updateCost(int newCost) { this->cost = newCost; }

    friend ostream & operator << (ostream &out, const Grid &g);
private:
    int rows;
    int columns;
    int ** grid;

    int cost;

    CoverageProblem * problem;


    void buildGrid(int m, int n);
    void addForbiddenPoints(CoverageProblem * problem);
    int getBlockSize(int type);
    bool horizontalBlock(Block * block, int length);
    bool verticalBlock(Block * block, int length);
    void updateCost(int type, int length, bool isAdded = true);
    bool isBlockValid(Block * block);
};

#endif //COVERAGE_GRID_H
