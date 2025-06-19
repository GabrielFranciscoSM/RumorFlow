#pragma once
#include <mh.h>
#include <population.h>
#include <problem.h>
#include <map>
#include <RF.h>

class RFG : public MH{

    const int NumOfGraphs = 5;
    
    vector<RF> populations;

public:
    RFG(Population::graphTypes graphType, Problem * problem);
    RFG(Population::graphTypes graphType, Problem * problem, float mp, double mi, float ps, int mr, double sp, float tr, float de);
    virtual ~RFG();

    ResultMH optimize(Problem * problem, int maxevals) override;
};