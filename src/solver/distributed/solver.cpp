//
// Created by Adam Zvada on 2019-04-30.
//

#include "solver.h"

Grid * Solver::solve() {

    Grid * grid = new Grid(problem);

    this->solutionGrid = new Grid(grid, problem);


    MPI_Init(nullptr, nullptr);

    solveMPI(grid);

//    // TODO: memory leak Point
//    # pragma omp parallel
//    {
//        # pragma omp single
//        {
//            //this->solveLoop(grid);
//            this->dfsRecursive(grid, new Point(0, 0), 0);
//        };
//    };

    cout << "LBC: " << solutionGrid->lowerBoundCost() << endl;
    cout << "UBC: " << solutionGrid->upperBoundCost(new Point(0, 0)) << endl;
    cout << "C: " << solutionGrid->getCost() << endl;

    return solutionGrid;
}

Solver::Solver(CoverageProblem * problem) {
    this->problem = problem;
    this->solutionGrid = nullptr;
}

Grid * Solver::solveMPI(Grid * grid) {

    // initial position
    Point * cord = new Point(0, 0);
    if (grid->getGridValue(cord->getX(), cord->getY()) == BLOCKED) {
        cord = this->nextCord(cord, grid);
    }

    queue<pair<Grid*, Point*>> q;

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {

        int numProcesses;
        MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);

        // Run BFS and feed the queue with first jobs
        q.push(make_pair(grid, cord));

        // generate numJobs and store it in queue
        while (q.size() < numProcesses) {

            auto stateJob = q.front();
            q.pop();

            // generate state jobs, it stores is in queue
            auto stateJobs = this->bfs(stateJob.first, stateJob.second, 1);
            delete stateJob.first;
            delete stateJob.second;

            // iterate over queue and add job states to the main job queue
            while (!stateJobs.empty()) {
                q.push(stateJobs.front());

                stateJobs.pop();
            }
        }

        // distribute jobs to all slaves
        for (int i = 1; i < numProcesses; i++) {
            pair<Grid*, Point*> stateJob = q.front();
            q.pop();

            vector<int> serializedJob = jobSerialization(stateJob.first, stateJob.second);
            delete stateJob.first;
            delete stateJob.second;

            // send index init workers?
            int jobSize = serializedJob.size();
            MPI_Send(&jobSize, 1, MPI_INT, i, TAG_INIT_SIZE, MPI_COMM_WORLD); // TAG_INIT - 0

            MPI_Send(serializedJob.data(), serializedJob.size(), MPI_INT, i, TAG_JOB, MPI_COMM_WORLD); // TAG_WORK - 1
        }

        MPI_Status mpiStatus;
        int workingSlaves = numProcesses - 1;
        while (workingSlaves > 0) {

            int jobResultSize;
            MPI_Recv(&jobResultSize, 1, MPI_INT, MPI_ANY_SOURCE, TAG_INIT_SIZE, MPI_COMM_WORLD, &mpiStatus);

            vector<int> jobResult;
            jobResult.resize(jobResultSize);
            MPI_Recv(&jobResult[0], jobResultSize, MPI_INT, mpiStatus.MPI_SOURCE, TAG_DONE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            pair<Grid*, Point*> resultJob = jobDeserialization(jobResult);

            if (resultJob.first->getCost() > this->solutionGrid->getCost()) {

                cout << "Found better Grid." << endl;
                cout << *resultJob.first;

                delete this->solutionGrid;
                this->solutionGrid = new Grid(resultJob.first, this->problem);
            }

            if (!q.empty()) {
                // send new job from queue
                pair<Grid*, Point*> stateJob = q.front();
                q.pop();

                vector<int> serializedJob = jobSerialization(stateJob.first, stateJob.second);
                delete stateJob.first;
                delete stateJob.second;

                int jobSize = serializedJob.size();
                MPI_Send(&jobSize , 1, MPI_INT, mpiStatus.MPI_SOURCE, TAG_INIT_SIZE, MPI_COMM_WORLD); // TAG_INIT - 0

                MPI_Send(serializedJob.data(), serializedJob.size(), MPI_INT, mpiStatus.MPI_SOURCE, TAG_JOB, MPI_COMM_WORLD); // TAG_WORK - 1
            } else {
                // Inform about finish
                MPI_Send(&workingSlaves, 1, MPI_INT, mpiStatus.MPI_SOURCE, TAG_FINISHED, MPI_COMM_WORLD);
                workingSlaves--;
            }
        }

    } else {

        bool endIndicator = false;
        MPI_Status mpiStatus;

        while (!endIndicator) {

            int jobSize;
            MPI_Recv(&jobSize, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &mpiStatus);

            if (mpiStatus.MPI_TAG != TAG_FINISHED) {
                // MPI_SOURCE should be MASTER!
                vector<int> job;
                job.resize(jobSize);
                MPI_Recv(&job[0], jobSize, MPI_INT, mpiStatus.MPI_SOURCE, TAG_JOB, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                pair<Grid*, Point*> jobState = jobDeserialization(job);

                Grid *jobResult;
# pragma omp parallel
                {
# pragma omp single
                    jobResult = dfsRecursive(jobState.first, jobState.second, 0);
                }
                vector<int> jobResultSerialized = jobSerialization(jobResult, jobState.second);
                //delete jobState.first;
                //delete jobState.second;
                //delete jobResult;

                cout << "Slave finished computation." << endl;

                int jobResultSize = jobResultSerialized.size();
                MPI_Send(&jobResultSize, 1, MPI_INT, mpiStatus.MPI_SOURCE, TAG_INIT_SIZE, MPI_COMM_WORLD);
                MPI_Send(jobResultSerialized.data(), jobResultSize, MPI_INT, mpiStatus.MPI_SOURCE, TAG_DONE, MPI_COMM_WORLD);
            } else {
                endIndicator = true;
            }
        }
    }

    MPI_Finalize();

    return this->solutionGrid;
}

queue<pair<Grid*, Point*>> Solver::bfs(Grid * grid, Point * cord, int depth) {

    Grid * curGrid = grid;
    Point * curCord = cord;

    queue<pair<Grid*, Point*>> q;

    while(q.size() < depth) {

        if (!q.empty()) {
            pair<Grid*, Point*> state = q.front();

            curGrid = state.first;
            curCord = this->nextCord(state.second, curGrid);

            q.pop();
        }

        vector<Block*> possibleBlocks = curGrid->generatePossibleBlocks(curCord);
        for (auto block : possibleBlocks) {

            if (curGrid->addBlockIfPossible(block)) {

                Grid *newGrid = new Grid(curGrid, problem);
                Point *newCord = new Point(curCord->getX(), curCord->getY());
                newCord = this->nextCord(newCord, newGrid);

                q.push(make_pair(newGrid, newCord));

                curGrid->undoBlock(block);
            }

        }
    }

    return q;
};


vector<int> Solver::jobSerialization(Grid * grid, Point * point) {

    int rows = this->problem->getRowSize();
    int columns = this->problem->getColumnSize();

    vector<int> serializedJob;

    serializedJob.push_back(point->getX());
    serializedJob.push_back(point->getY());

    serializedJob.push_back(grid->getCost());

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            serializedJob.push_back(grid->getGridValue(i, j));
        }
    }

    return serializedJob;

}

pair<Grid*, Point*> Solver::jobDeserialization(vector<int> & serializedJob) {

    int rows = this->problem->getRowSize();
    int columns = this->problem->getColumnSize();

    Point * cord = new Point(serializedJob[0], serializedJob[1]);
    Grid * grid = new Grid(this->problem);
    grid->updateCost(serializedJob[2]);

    int counter = 3;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            grid->updateGridValue(i, j, serializedJob[counter]);
            counter++;
        }
    }

    return make_pair(grid, cord);
}


Grid * Solver::solveLoop(Grid * grid) {

    int depth = 10;

    queue<pair<Grid*, Point*>> q;

    // initial position
    Point * cord = new Point(0, 0);
    if (grid->getGridValue(cord->getX(), cord->getY()) == BLOCKED) {
        cord = this->nextCord(cord, grid);
    }

    Grid * curGrid = grid;
    Point * curCord = cord;
    while(q.size() < depth) {

        if (!q.empty()) {
            pair<Grid*, Point*> state = q.front();

            curGrid = state.first;
            curCord = this->nextCord(state.second, curGrid);

            q.pop();
        }

        vector<Block*> possibleBlocks = curGrid->generatePossibleBlocks(curCord);
        for (auto block : possibleBlocks) {

            if (curGrid->addBlockIfPossible(block)) {

                Grid *newGrid = new Grid(curGrid, problem);
                Point *newCord = new Point(0, 0);

                cout << *newGrid << endl;

                q.push(make_pair(newGrid, newCord));

                curGrid->undoBlock(block);
            }

        }
    }

    //#pragma omp parallel for //default(shared)
    for (int i = 0; i < q.size(); i++) {

        pair<Grid*, Point*> jobState = q.front();
        q.pop();

        dfsRecursive(jobState.first, jobState.second, 0);
    }

    return this->solutionGrid;
}

Grid * Solver::dfsRecursive(Grid * grid, Point * cord, int depth) {

    if (cord == nullptr) {
        return this->solutionGrid;
    }

    vector<Block*> possibleBlocks = grid->generatePossibleBlocks(cord);
    if (possibleBlocks.size() == 0) {

        Point * nextCord = this->nextCord(cord, grid);

        if (grid->upperBoundCost(nextCord) + grid->getCostWithoutPenalty(nextCord) > this->solutionGrid->getCost()) {
            Grid * newGrid = new Grid(grid, problem);
# pragma omp task if (depth < THRESHOLD)
            {
                this->dfsRecursive(newGrid, nextCord, ++depth);
            };
        }

        delete cord;

        return this->solutionGrid;
    }

    //for (auto block : possibleBlocks) {
    //#pragma omp parallel for //default(shared)
    for (int i = 0; i < possibleBlocks.size(); i++) {

        bool isAdded = grid->addBlockIfPossible(possibleBlocks[i]);

        if (isAdded) {

#pragma omp critical
            {
                if (grid->getCost() > this->solutionGrid->getCost()) {
                    delete this->solutionGrid;
                    this->solutionGrid = new Grid(grid, this->problem);
                }
            };

            Point *nextCord = this->nextCord(cord, grid);

            if (grid->upperBoundCost(nextCord) + grid->getCostWithoutPenalty(nextCord) > this->solutionGrid->getCost()) {
                Grid * newGrid = new Grid(grid, problem);
# pragma omp task if (depth < THRESHOLD)
                {
                    this->dfsRecursive(newGrid, nextCord, ++depth);
                };
            }

            if (possibleBlocks[i]->getType() != EMPTY) {
                grid->undoBlock(possibleBlocks[i]);
            }
        }
    }

    delete cord;
    delete grid;

    return this->solutionGrid;
}

Point * Solver::nextCord(Point * cord, Grid * grid) {

    int rows = this->problem->getRowSize();
    int columns = this->problem->getColumnSize();

    int x = cord->getX();
    int y = cord->getY();
    //delete cord;

    do {
        if (x + 1 < rows) {
            x++;
            continue;
        }

        if (y + 1 < columns) {
            x = 0;
            y++;
            continue;
        }
    } while (grid->getGridValue(x,y) != 0 && !(x + 1 == rows && y + 1 == columns));

    if (x + 1 == rows && y + 1 == columns) {
        return nullptr;
    }

    return new Point(x, y);
}
