//
// Created by Adam Zvada on 2019-04-30.
//
#ifndef COVERAGE_COVERAGE_PROBLEM_H
#define COVERAGE_COVERAGE_PROBLEM_H

#include <vector>

#include "point.h"

using namespace std;

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

    friend istream & operator >> (istream &in, CoverageProblem &c);
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





#endif //COVERAGE_COVERAGE_PROBLEM_H
