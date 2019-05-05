//
// Created by Adam Zvada on 2019-04-30.
//

#ifndef COVERAGE_BLOCK_H
#define COVERAGE_BLOCK_H


#include "point.h"

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


#endif //COVERAGE_BLOCK_H
