//
// Created by Adam Zvada on 2019-04-30.
//

#ifndef COVERAGE_POINT_H
#define COVERAGE_POINT_H

#include <istream>

using namespace std;

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

#endif //COVERAGE_POINT_H
