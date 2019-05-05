//
// Created by Adam Zvada on 2019-04-30.
//

#include "grid.h"


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

    Block * emptyBlock = new Block(new Point(*cord), EMPTY, EMPTY, 0);
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

    delete block;

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
