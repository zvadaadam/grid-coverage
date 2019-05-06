#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include <omp.h>
#include <chrono>
#include <queue>
#include <mpi.h>


#define BLOCKED -1

#define EMPTY 0

#define TYPE_1 1
#define TYPE_2 2

#define HORIZONTAL 3
#define VERTICAL 4

#define THRESHOLD 10

using namespace std;

typedef std::chrono::high_resolution_clock Clock;

//______________________________________________________________

// TODO: destructor
class Point {
public:
    Point(int x = 0, int y = 0);
    Point(const Point & p);

    int getX() { return this->x; }
    int getY() { return this->y; }

    friend istream & operator >> (istream &in,  Point &p);
    friend ostream & operator << (ostream &out, const Point &p);
private:
    int x;
    int y;
};

Point::Point(int x, int y) {
    this->x = x;
    this->y = y;
}

Point::Point(const Point & p) {
    this->x = p.x;
    this->y = p.y;
}

istream & operator >> (istream &in,  Point &p)  {

    in >> p.y;
    in >> p.x;

    return in;
}

ostream & operator << (ostream &out, const Point &p) {

    out << p.x << ", " << p.y << endl;

    return out;
}

//______________________________________________________________

class CoverageProblem {
public:
    CoverageProblem() { m = 0; n = 0; }

    int getRowSize() { return m; }
    int getColumnSize() { return n; }
    int getI1Length() { return i1Length; }
    int getI2Length() { return i2Length; }
    int getI1Cost() { return i1Cost; }
    int getI2Cost() { return i2Cost; }
    int getPenalization() { return penalization; }

    vector<Point> &getForbiddenPoints() { return this->forbiddenPoints; }

    friend istream & operator >> (istream &in,  CoverageProblem &c);
    friend ostream & operator << (ostream &out, const CoverageProblem &c);
private:
    int m;
    int n;

    int i1Length;
    int i1Cost;
    int i2Length;
    int i2Cost;
    int penalization;

    vector<Point> forbiddenPoints;

    int calculateUpperBoundPrice();
};

istream & operator >> (istream &in,  CoverageProblem &c)  {

    in >> c.m;
    in >> c.n;

    in >> c.i1Length;
    in >> c.i2Length;
    in >> c.i1Cost;
    in >> c.i2Cost;
    in >> c.penalization;

    int numForbidden;
    in >> numForbidden;

    for (int i = 0; i < numForbidden; ++i) {
        Point point;
        in >> point;
        c.forbiddenPoints.push_back(point);
    }

    return in;
}

ostream & operator << (ostream &out, const CoverageProblem &c) {

    out << "Matrix size: ";
    out << c.m << ", " << c.n << endl;

    out << "Length of I1 " << c.i1Length << " with cost " << c.i1Cost << endl;
    out << "Length of I2 " << c.i2Length << " with cost " << c.i2Cost << endl;
    out << "Penalization of uncovered block: " << c.penalization << endl;

    out << "Forbidden Points " << "(" << c.forbiddenPoints.size() << "): " << endl;
    for(auto && x: c.forbiddenPoints) {
        out << x;
    }

    return out;
}

//int CoverageProblem::calculateUpperBoundPrice() {
//
//    int numCells = m * n - forbiddenPoints.size();
//
//    return numCells * i2Cost
//};

//______________________________________________________________

class Block {
public:
    Block(Point * cord, int type, int orientation, int id);
    ~Block();

    Point * getCord() { return this->cord; };
    int getType() { return this->type; };
    int getOrientation() { return this->orientation; };
    int getId() { return this->id; };
private:
    Point * cord;
    int type;
    int orientation;
    int id;
};

Block::Block(Point * cord, int type, int orientation, int id) {

    this->cord = new Point(cord->getX(), cord->getY());
    this->type = type;
    this->orientation = orientation;
    this->id = id;
}

Block::~Block() {
    delete cord;
}


//______________________________________________________________


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

Grid::Grid(CoverageProblem * problem) {
    this->problem = problem;

    this->rows = this->problem->getRowSize();
    this->columns = this->problem->getColumnSize();

    this->buildGrid(this->rows, this->columns);
    this->addForbiddenPoints(problem);

    // calculate initial cost "penalization"
    int penalization = this->problem->getPenalization();
    this->cost = (((this->rows * this->columns) - problem->getForbiddenPoints().size())*penalization);
}

Grid::Grid(Grid * grid, CoverageProblem * problem) {

    this->problem = problem;

    this->rows = this->problem->getRowSize();
    this->columns = this->problem->getColumnSize();

    this->cost = grid->getCost();

    this->buildGrid(this->rows, this->columns);

    // TODO: faster copy!
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            this->grid[i][j] = grid->getGridValue(i, j);
        }
    }
}

Grid::~Grid() {

    for (int i = 0; i < this->rows; i++) {
        delete[] grid[i];
    }
    delete[] grid;
}

int Grid::getCostWithoutPenalty(Point * cord) {

    int unsolvedSquares = 0;
    if (cord != NULL) {
        int x = cord->getX();
        int y = cord->getY();

        for (int i = x; i < rows; i++) {
            if (this->grid[i][y] == 0) {
                unsolvedSquares++;
            }
        }

        for (int i = 0; i < rows; ++i) {
            for (int j = y + 1; j < columns; ++j) {
                if (this->grid[i][j] == 0) {
                    unsolvedSquares++;
                }
            }
        }
    } else {
        unsolvedSquares = 0;
    }

    return getCost() - problem->getPenalization()*unsolvedSquares;
}

bool Grid::addBlockIfPossible(Block * block) {

    int x = block->getCord()->getX();
    int y = block->getCord()->getY();

    int blockSize = this->getBlockSize(block->getType());

    // blockSize == 0
    if (x >= this->rows || y >= this->columns || y < 0 || x < 0) {
        return false;
    }

    if (block->getOrientation() == EMPTY) {
        return true;
    }

    if (block->getOrientation() == HORIZONTAL) {
        return this->horizontalBlock(block, blockSize);
    }

    if (block->getOrientation() == VERTICAL) {
        return this->verticalBlock(block, blockSize);
    }

    return false;
}

//int Grid::addBlock(Point * cord, int id, int type, bool isHorizontal) {
//
//    int x = cord->getX();
//    int y = cord->getY();
//
//    int blockSize = this->getBlockSize(type);
//
//    if (blockSize == 0 || x >= this->rows || y >= this->columns || y < 0 || x < 0 ||
//            x + blockSize > this->rows || y + blockSize > this->columns) {
//        return -1;
//    }
//
//    return isHorizontal ? this->horizontalBlock(cord, id, blockSize) : this->verticalBlock(cord, id, blockSize);
//}

vector<Block*> Grid::generatePossibleBlocks(Point * cord) {

    vector<Block*> possibleBlocks;

    int blockSizeI1 = this->getBlockSize(TYPE_1);
    int blockSizeI2 = this->getBlockSize(TYPE_2);

//    cout << "generatePossibleBlocks" << endl;
//    cout << "I1: " << blockSizeI1 << endl;
//    cout << "I2: " << blockSizeI2 << endl;
//    cout << "Cord: [" << cord->getX() << ", " << cord->getY() << "]" << endl;
//    cout << *this;

    if (this->getGridValue(cord->getX(), cord->getY()) == BLOCKED || this->getGridValue(cord->getX(), cord->getY()) > 0) {
        return possibleBlocks;
    }

    // I1 - horizontal
    if (cord->getY() + blockSizeI1 - 1 < this->columns) {
        Block * newBlock = new Block(cord, TYPE_1, HORIZONTAL, 2);
        if (this->isBlockValid(newBlock)) {
            possibleBlocks.push_back(newBlock);
        } else {
            delete newBlock;
        }
    }

    // TODO: Add IDs
    // I1 - vertical
    if (cord->getX() + blockSizeI1 - 1 < this->rows) {
        Block * newBlock = new Block(cord, TYPE_1, VERTICAL, 1);
        if (this->isBlockValid(newBlock)) {
            possibleBlocks.push_back(newBlock);
        } else {
            delete newBlock;
        }
    }

    // I2 - vertical
    if (cord->getX() + blockSizeI2 - 1 < this->rows) {
        Block * newBlock = new Block(cord, TYPE_2, VERTICAL, 3);
        if (this->isBlockValid(newBlock)) {
            possibleBlocks.push_back(newBlock);
        } else {
            delete newBlock;
        }
    }

    // I2 - horizontal
    if (cord->getY() + blockSizeI2 - 1 < this->columns) {
        Block * newBlock = new Block(cord, TYPE_2, HORIZONTAL, 4);
        if (this->isBlockValid(newBlock)) {
            possibleBlocks.push_back(newBlock);
        } else {
            delete newBlock;
        }
    }

    Block * emptyBlock = new Block(cord, EMPTY, EMPTY, 0);
    possibleBlocks.push_back(emptyBlock);

    return possibleBlocks;
}

bool Grid::isBlockValid(Block * block) {


    int x = block->getCord()->getX();
    int y = block->getCord()->getY();
    int orientation = block->getOrientation();
    int blockSize = this->getBlockSize(block->getType());

    for (int i = 0; i < blockSize; ++i) {
        if (orientation == HORIZONTAL) {
            if (this->grid[x][y + i] != 0) {
                return false;
            }
        } else if (orientation == VERTICAL) {
            if (this->grid[x + i][y] != 0) {
                return false;
            }
        } else {
            throw orientation;
        }
    }


    return true;
}

ostream & operator << (ostream &out, const Grid &g) {


    out << "[" << g.rows << ", " << g.columns<< "]" << endl;

    for (int i = 0; i < g.rows; ++i) {
        for (int j = 0; j < g.columns; ++j) {
            out << g.grid[i][j] << " ";
        }
        out << endl;
    }

    out << "Cost: " << g.cost << endl;

    return out;
}


void Grid::buildGrid( int rows, int columns) {

    this->grid = new int*[rows];

    for (int i = 0; i < rows; ++i) {
        this->grid[i] = new int[columns];
    }

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            this->grid[i][j] = 0;
        }
    }
}

void Grid::addForbiddenPoints(CoverageProblem * problem) {

    for(auto && point: problem->getForbiddenPoints()) {
        int x = point.getX();
        int y = point.getY();

        this->grid[x][y] = BLOCKED;
    }
}

int Grid::getBlockSize(int type) {

    if (type == TYPE_1) {
        return this->problem->getI1Length();
    } else if (type == TYPE_2) {
        return this->problem->getI2Length();
    }

    return 0;
}

bool Grid::horizontalBlock(Block * block, int length) {

    int x = block->getCord()->getX();
    int y = block->getCord()->getY();
    int id = block->getId();
    int type = block->getType();

    if (y + length - 1 > this->columns) {
        return false;
    }

    for (int i = 0; i < length; ++i) {
        if (this->grid[x][y + i] != 0) {
            return false;
        }
    }

    for (int i = 0; i < length; ++i) {
        this->grid[x][y + i] = id;
    }


    this->updateCost(type, length, true);


    return true;
}


bool Grid::verticalBlock(Block * block, int length) {

    int x = block->getCord()->getX();
    int y = block->getCord()->getY();
    int id = block->getId();
    int type = block->getType();

    if (x + length - 1 > this->rows) {
        return false;
    }

    for (int i = 0; i < length; i++) {
        if (this->grid[x + i][y] != 0) {
            return false;
        }
    }

    for (int i = 0; i < length; i++) {
        this->grid[x + i][y] = id;
    }

    this->updateCost(type, length, true);


    return true;
}

void Grid::updateCost(int type, int length, bool isAdded) {

    int blockCost = 0;
    if (type == TYPE_1) {
        blockCost = this->problem->getI1Cost();
    } else if (type == TYPE_2) {
        blockCost = this->problem->getI2Cost();
    }
    int penalization = length*this->problem->getPenalization();

    if (isAdded) {
        this->cost -= penalization;
        this->cost += blockCost;
    } else {
        this->cost += penalization;
        this->cost -= blockCost;
    }
}

bool Grid::undoBlock(Block * block) {

    int x = block->getCord()->getX();
    int y = block->getCord()->getY();
    int orientation = block->getOrientation();

    int blockSize = this->getBlockSize(block->getType());

    for (int i = 0; i < blockSize; ++i) {
        if (this->grid[x][y] != block->getId()) {
            throw 42; // TODO: Beter exception
        }

        this->grid[x][y] = 0;

        if (orientation == VERTICAL) {
            x++;
        } else {
            y++;
        }
    }
    // reduced the cost
    this->updateCost(block->getType(), blockSize, false);

    return true;
}

int Grid::lowerBoundCost() {
    return problem->getPenalization() * (rows * columns - problem->getForbiddenPoints().size());
}

int Grid::upperBoundCost(Point * cord) {

    // vraci maximalni cenu pro "number" nevyresenych policek
    // returns the maximal price for the "number" of unsolved squares

    int unsolvedSquares = 0;
    if (cord != NULL) {
        int x = cord->getX();
        int y = cord->getY();

        for (int i = x; i < rows; i++) {
            if (this->grid[i][y] == 0) {
                unsolvedSquares++;
            }
        }

        for (int i = 0; i < rows; ++i) {
            for (int j = y + 1; j < columns; ++j) {
                if (this->grid[i][j] == 0) {
                    unsolvedSquares++;
                }
            }
        }
    }

    int i1Cost = problem->getI1Cost();
    int i2Cost = problem->getI2Cost();

    int i1Size = problem->getI1Length();
    int i2Size = problem->getI2Length();

    int penalty = problem->getPenalization();

    int max = i2Cost * (unsolvedSquares/i2Size);
    int reminder = unsolvedSquares % i2Size;

    max += i1Cost * (reminder/i1Size);
    reminder = reminder % i1Size;
    max += reminder * penalty;

    int val = 0;
    for (int i = 0; i < (unsolvedSquares/i2Size); i++) {

        val = i2Cost * i;

        reminder = unsolvedSquares - (i * i2Size);
        val += i2Cost * (reminder/i2Size);

        reminder = reminder % i1Size;
        val += reminder * penalty;

        if (val > max) {
            max = val;
        }
    }

    return max;
}

//______________________________________________________________


class Solver {
public:
    Solver(CoverageProblem * problem);

    Grid * solveDistributed();
    Grid * solveSequence();
    Grid * solveTaskParallel(int depthThreshold);
    Grid * solveDataParallel(int depth);
private:
    CoverageProblem * problem;
    Grid * solutionGrid;

    Grid * solveMPI(Grid * grid);
    Grid * solveLoop(Grid * grid, int depth);

    queue<pair<Grid*, Point*>> bfs(Grid * grid, Point * cord, int depth);
    Grid * dfsRecursive(Grid * grid, Point * cord, int depth);
    Grid * dfsRecursiveTask(Grid * grid, Point * cord, int depth, const int depthThreshold);

    vector<int> jobSerialization(Grid * grid, Point * point);
    pair<Grid*, Point*> jobDeserialization(vector<int> & serializedJob);

    Point * nextCord(Point * cord, Grid * grid);

    const int TAG_INIT_SIZE = 0;
    const int TAG_JOB = 1;
    const int TAG_RESULT= 2;
    const int TAG_DONE = 3;
    const int TAG_FINISHED = 4;
};

Solver::Solver(CoverageProblem * problem) {
    this->problem = problem;
    this->solutionGrid = nullptr;
}

Grid * Solver::solveSequence() {

    Grid * grid = new Grid(problem);

    this->solutionGrid = new Grid(grid, problem);
    Point * initCord = new Point(0, 0);

    this->dfsRecursive(grid, initCord, 0);

//    cout << "LBC: " << solutionGrid->lowerBoundCost() << endl;
//    cout << "UBC: " << solutionGrid->upperBoundCost(initCord) << endl;
    cout << "C: " << solutionGrid->getCost() << endl;

    delete grid;
    delete initCord;

    return solutionGrid;
}

Grid * Solver::solveTaskParallel(int depthThreshold) {

    Grid * grid = new Grid(problem);

    this->solutionGrid = new Grid(grid, problem);
    Point * initCord = new Point(0, 0);

    // TODO: memory leak Point
    # pragma omp parallel
    {
        # pragma omp single
        {
            this->dfsRecursiveTask(grid, initCord, 0, depthThreshold);
        };
    };

//    cout << "LBC: " << solutionGrid->lowerBoundCost() << endl;
//    cout << "UBC: " << solutionGrid->upperBoundCost(new Point(0, 0)) << endl;
    cout << "C: " << solutionGrid->getCost() << endl;

    delete initCord;

    return solutionGrid;
}

Grid * Solver::solveDataParallel(int depth) {

    Grid * grid = new Grid(problem);

    this->solutionGrid = new Grid(grid, problem);

    this->solveLoop(grid, depth);

//    cout << "LBC: " << solutionGrid->lowerBoundCost() << endl;
//    cout << "UBC: " << solutionGrid->upperBoundCost(new Point(0, 0)) << endl;
    cout << "C: " << solutionGrid->getCost() << endl;

    return solutionGrid;
}

Grid * Solver::solveDistributed() {
    Grid * grid = new Grid(problem);

    this->solutionGrid = new Grid(grid, problem);

    MPI_Init(nullptr, nullptr);
    this->solveMPI(grid);

    cout << "LBC: " << solutionGrid->lowerBoundCost() << endl;
    cout << "UBC: " << solutionGrid->upperBoundCost(new Point(0, 0)) << endl;
    cout << "C: " << solutionGrid->getCost() << endl;

    return solutionGrid;
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
                    jobResult = dfsRecursiveTask(jobState.first, jobState.second, 0, 15);
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


Grid * Solver::solveLoop(Grid * grid, int depth) {

    queue<pair<Grid*, Point*>> q;

    // initial position
    Point * curCord = new Point(0, 0);
    if (grid->getGridValue(curCord->getX(), curCord->getY()) == BLOCKED) {
        Point* nextCord = this->nextCord(curCord, grid);

        delete curCord;
        curCord = nextCord;
    }

    Grid * curGrid = grid;
    while(q.size() < depth) {

        if (!q.empty()) {
            pair<Grid*, Point*> state = q.front();

            delete curGrid;
            curGrid = state.first;

            curCord = this->nextCord(state.second, curGrid);

            q.pop();

            delete state.second;
        }

        vector<Block*> possibleBlocks = curGrid->generatePossibleBlocks(curCord);
        for (auto block : possibleBlocks) {

            if (curGrid->addBlockIfPossible(block)) {

                Grid *newGrid = new Grid(curGrid, problem);
                Point *newCord = this->nextCord(curCord, curGrid);

                q.push(make_pair(newGrid, newCord));

                curGrid->undoBlock(block);
            }

        }
    }

    cout << "Generated jobs: " <<  q.size() << endl;

    #pragma omp parallel for shared(q)
    for (int i = 0; i < q.size(); i++) {
        pair<Grid *, Point *> jobState;
        # pragma omp critical
        {
            jobState = q.front();
            q.pop();
        }

        dfsRecursive(jobState.first, jobState.second, 0);

        delete jobState.first;
        delete jobState.second;
    }

    return this->solutionGrid;
}

Grid * Solver::dfsRecursiveTask(Grid * grid, Point * cord, int depth, const int depthThreshold) {

    if (cord == nullptr) {
        return this->solutionGrid;
    }

    vector<Block*> possibleBlocks = grid->generatePossibleBlocks(cord);
    if (possibleBlocks.size() == 0) {

        Point * nextCord = this->nextCord(cord, grid);

        if (grid->upperBoundCost(nextCord) + grid->getCostWithoutPenalty(nextCord) > this->solutionGrid->getCost()) {

            Grid * nextGrid = new Grid(grid, problem);

            #pragma omp task //if (depth < depthThreshold)
            this->dfsRecursiveTask(nextGrid, nextCord, ++depth, depthThreshold);
        }
        //delete nextCord;

        return this->solutionGrid;
    }

    for (int i = 0; i < possibleBlocks.size(); i++) {

        bool isAdded = grid->addBlockIfPossible(possibleBlocks[i]);

        if (isAdded) {

            # pragma omp critical
            {
                if (grid->getCost() > this->solutionGrid->getCost()) {
                    delete this->solutionGrid;
                    this->solutionGrid = new Grid(grid, this->problem);
                }
            };

            Point *nextCord = this->nextCord(cord, grid);

            if (grid->upperBoundCost(nextCord) + grid->getCostWithoutPenalty(nextCord) > this->solutionGrid->getCost()) {
                Grid * nextGrid = new Grid(grid, problem);

                # pragma omp task //if (depth < depthThreshold)
                this->dfsRecursiveTask(nextGrid, nextCord, ++depth, depthThreshold);
            }
            //delete nextCord;

            if (possibleBlocks[i]->getType() != EMPTY) {
                grid->undoBlock(possibleBlocks[i]);
            }
        }

        delete possibleBlocks[i];
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
            this->dfsRecursive(newGrid, nextCord, ++depth);

            delete newGrid;
        }
        delete nextCord;

        return this->solutionGrid;
    }

    for (int i = 0; i < possibleBlocks.size(); i++) {

        bool isAdded = grid->addBlockIfPossible(possibleBlocks[i]);

        if (isAdded) {

            # pragma omp critical
            {
                if (grid->getCost() > this->solutionGrid->getCost()) {
                    delete this->solutionGrid;
                    this->solutionGrid = new Grid(grid, this->problem);
                }
            };

            Point *nextCord = this->nextCord(cord, grid);

            if (grid->upperBoundCost(nextCord) + grid->getCostWithoutPenalty(nextCord) > this->solutionGrid->getCost()) {
                Grid * newGrid = new Grid(grid, problem);
                this->dfsRecursive(newGrid, nextCord, ++depth);

                delete newGrid;
            }
            delete nextCord;

            if (possibleBlocks[i]->getType() != EMPTY) {
                grid->undoBlock(possibleBlocks[i]);
            }
        }
        delete possibleBlocks[i];
    }

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


//______________________________________________________________

int main(int argc,  char **argv) {

    if(argc != 4) {
        cout << "Missing input file or solver type" << endl;
        return 1;
    }

    cout << "Processing file: " << argv[1] << endl;

    ifstream fs(argv[1], ios::in);

    int solverType = strtol(argv[2], NULL, 10);
    int depthThreshold = strtol(argv[3], NULL, 10);

    CoverageProblem * problem = new CoverageProblem();
    fs >> *problem;

    //cout << *problem;

    Solver * solver = new Solver(problem);

    auto start = chrono::high_resolution_clock::now();

    if (solverType == 0) {
        cout << "Sequence" << endl;
        cout << *solver->solveSequence();
    } else if (solverType == 1) {
        cout << "Task Parallel" << endl;
        cout << *solver->solveTaskParallel(depthThreshold);
    } else if (solverType == 2) {
        cout << "Data Parallel" << endl;
        cout << *solver->solveDataParallel(depthThreshold);
    } else if (solverType == 3) {
        cout << "Distributed" << endl;
        cout << *solver->solveDistributed();
    } else {
        cout << "Unsupported solver type." << endl;
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, std::ratio<1>> elapsed = end-start;
    cout << "Program duration: " << elapsed.count() << " seconds" << std::endl;

    delete problem;
    delete solver;

    return 0;
}