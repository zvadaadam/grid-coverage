//
// Created by Adam Zvada on 2019-04-30.
//

#ifndef COVERAGE_SOLVER_H
#define COVERAGE_SOLVER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include <omp.h>
#include <chrono>
#include <queue>
#include <mpi.h>

#include "../../model/grid.h"
#include "../../model/coverage_problem.h"

#define THRESHOLD 10

class Solver {
public:
    Solver(CoverageProblem * problem);

    Grid * solve();
private:
    CoverageProblem * problem;
    Grid * solutionGrid;

    Grid * solveMPI(Grid * grid);
    Grid * solveLoop(Grid * grid);

    queue<pair<Grid*, Point*>> bfs(Grid * grid, Point * cord, int depth);
    Grid * dfsRecursive(Grid * grid, Point * cord, int depth);

    vector<int> jobSerialization(Grid * grid, Point * point);
    pair<Grid*, Point*> jobDeserialization(vector<int> & serializedJob);

    Point * nextCord(Point * cord, Grid * grid);

    const int TAG_INIT_SIZE = 0;
    const int TAG_JOB = 1;
    const int TAG_RESULT= 2;
    const int TAG_DONE = 3;
    const int TAG_FINISHED = 4;
};

#endif //COVERAGE_SOLVER_H
