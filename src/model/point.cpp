//
// Created by Adam Zvada on 2019-04-30.
//

#include "point.h"


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
