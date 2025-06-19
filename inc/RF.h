#pragma once
#include <mh.h>
#include <population.h>
#include <problem.h>
#include <map>
#include <fstream>
#include <filesystem>

class RF : public MH{

    const float MUTATE_PTOB;                      //0.05  0.1     0.2     0.5
    const double MUTATE_INTENSITY;               //0.5   0.75    1       1.25
    const double SELECT_PERCENTAGE;              //0.05  0.1     0.2     0.5
    const float POP_SIZE;                         //50    100     150
    const int MAX_ROUNDS;                           //1     2       3
    
    // Store divergence history as pairs of {evaluation_count, divergence_value}
    vector<pair<int, double>> divergenceHistory;
    
    const int SELECT_SIZE;

    const float TOURNAMENT_RATIO = 1;                   //0.1   0.5     1

    std::uniform_int_distribution<int> dist_PopSize;

    int evals;

public:
    Population pop;                                     

    int maxevals;
    bool LS;

    int rejectCounter = 0;

    RF(Population pop, Problem * problem);
    RF(Population::graphTypes graphType, Problem * problem);
    RF(Population::graphTypes graphType, Problem * problem, float mp, double mi, float ps, int mr, double sp, float tr, float de);
    
    ResultMH optimize(Problem * problem, int maxevals);
    ResultMH optimize(Problem * problem, int maxevals, bool LocalSearch);

    int select();
    std::map<tFitness, int> propagate(vector<int> & cromosomes, int Nrounds, Problem * problem);
    bool cross(Problem * problem, int spreader, int listener);
    void mutate(Problem * problem);
    tFitness mutateSol(Problem * problem, tSolution & sol, int range);

    void localSearch(Problem * problem, int cromosome, int evals);

    tFitness getFitness(tSolution & sol, Problem * problem);

    double divergence(Problem * prob);
    vector<double> deviation(Problem * prob);

    // Divergence data collection
    void saveDivergenceData(const string& funcName) const;
};