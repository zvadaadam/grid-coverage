#include <iostream>
#include <fstream>
#include <vector>
#include <stack>

#define BLOCKED -1
#define TYPE_1 1
#define TYPE_2 2

#define HORIZONTAL 3
#define VERTICAL 4

using namespace std;

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

    int upperBoundPrice() { return this->upperBoundPrice; }

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

    int upperBoundPrice;

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

int CoverageProblem::calculateUpperBoundPrice() {

    int numCells = m * n - forbiddenPoints.size();

    return numCells * i2
};

//______________________________________________________________

class Block {
public:
    Block(Point * cord, int type, int orientation, int id);
    //~Block();

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

    this->cord = cord;
    this->type = type;
    this->orientation = orientation;
    this->id = id;
}

//Block::~Block() {
//    delete cord;
//}


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
    int getGridValue(int i, int j) { return this->grid[i][j]; }

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

bool Grid::addBlockIfPossible(Block * block) {

    int x = block->getCord()->getX();
    int y = block->getCord()->getY();

    int blockSize = this->getBlockSize(block->getType());

    if (blockSize == 0 || x >= this->rows || y >= this->columns || y < 0 || x < 0) {
        return false;
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
        Block * newBlock = new Block(new Point(*cord), TYPE_1, HORIZONTAL, 2);
        if (this->isBlockValid(newBlock)) {
            possibleBlocks.push_back(newBlock);
        } else {
            delete newBlock;
        }
    }

    // TODO: Add IDs
    // I1 - vertical
    if (cord->getX() + blockSizeI1 - 1 < this->rows) {
        Block * newBlock = new Block(new Point(*cord), TYPE_1, VERTICAL, 1);
        if (this->isBlockValid(newBlock)) {
            possibleBlocks.push_back(newBlock);
        } else {
            delete newBlock;
        }
    }

    // I2 - vertical
    if (cord->getX() + blockSizeI2 - 1 < this->rows) {
        Block * newBlock = new Block(new Point(*cord), TYPE_2, VERTICAL, 3);
        if (this->isBlockValid(newBlock)) {
            possibleBlocks.push_back(newBlock);
        } else {
            delete newBlock;
        }
    }

    // I2 - horizontal
    if (cord->getY() + blockSizeI2 - 1 < this->columns) {
        Block * newBlock = new Block(new Point(*cord), TYPE_2, HORIZONTAL, 4);
        if (this->isBlockValid(newBlock)) {
            possibleBlocks.push_back(newBlock);
        } else {
            delete newBlock;
        }
    }

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

    delete block;

    return true;
}


//______________________________________________________________


class Solver {
public:
    Solver(CoverageProblem * problem);

    Grid * solve();
private:
    CoverageProblem * problem;
    Grid * solutionGrid;

    void dfsRecursive(Grid * grid, Point * cord, int depth);
    //Grid * dfsIterative(Grid * grid);

    Point * nextCord(Point * cord, Grid * grid);
};

Grid * Solver::solve() {

    Grid * grid = new Grid(problem);

    this->solutionGrid = new Grid(grid, problem);

    // TODO: memory leak Point
    this->dfsRecursive(grid, new Point(0, 0), 0);

    return solutionGrid;
}

Solver::Solver(CoverageProblem * problem) {
    this->problem = problem;
    this->solutionGrid = nullptr;
}


void Solver::dfsRecursive(Grid * grid, Point * cord, int depth) {

    if (cord == nullptr) {
        return;
    }

    vector<Block*> possibleBlocks = grid->generatePossibleBlocks(cord);
    if (possibleBlocks.size() == 0) {

//        cout << endl;
//        cout << "Depth: " << depth << endl;
//        cout << "NO generated blocks..." << endl;

        Point * nextCord = this->nextCord(cord, grid);
        this->dfsRecursive(grid, nextCord, ++depth);

        delete cord;

        return;
    }

    for (auto block : possibleBlocks) {

        bool isAdded = grid->addBlockIfPossible(block);

//        cout << endl;
//        cout << "Depth: " << depth << endl;
//        cout << "Cord: [" << cord->getX() << ", " << cord->getY() << "]" << endl;

        if (isAdded) {

            if (grid->getCost() > this->solutionGrid->getCost()) {
                delete this->solutionGrid;

                this->solutionGrid = new Grid(grid, this->problem);
                cout << *grid;
            }

            Point *nextCord = this->nextCord(cord, grid);
            this->dfsRecursive(grid, nextCord, ++depth);

            grid->undoBlock(block);
        }
    }

    delete cord;

    return;
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

    if(argc != 2) {
        cout << "Missing input file." << endl;
        return 1;
    }

    cout << "Processing file: " << argv[1] << endl;

    ifstream fs(argv[1], ios::in);

    CoverageProblem * problem = new CoverageProblem();
    fs >> *problem;

    cout << *problem;

    //--------TEST---------
//    Grid * grid = new Grid(&problem);
//
//    cout << grid->addBlockIfPossible(new Block(new Point(6,4), TYPE_1, HORIZONTAL, TYPE_1 + HORIZONTAL)) << endl;
//
//    cout << *grid;
    //--------TEST---------

    Solver * solver = new Solver(problem);

    cout << *solver->solve();

    return 0;
}