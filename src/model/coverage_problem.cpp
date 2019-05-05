//
// Created by Adam Zvada on 2019-04-30.
//

#include "coverage_problem.h"

istream & operator >> (istream &in, CoverageProblem &c)  {

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
