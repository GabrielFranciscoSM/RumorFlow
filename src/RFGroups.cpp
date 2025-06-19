#include <RFGroups.h>
#include <util.h>

RFG::RFG(Population::graphTypes graphType, Problem * problem): 
    MH() {
    for(int i = 0; i < NumOfGraphs; ++i){
        RF rf = RF(graphType, problem, 0.2, 0.75, 0.05, 100, 2, 100*0.05, 0.1);
        populations.push_back(rf);
    }
}

RFG::RFG(Population::graphTypes graphType, Problem * problem, float mp, double mi, float ps, int mr, double sp, float tr, float density): 
    MH() {
    for(int i = 0; i < NumOfGraphs; ++i){
        RF rf = RF(graphType, problem, mp, mi, ps, mr, sp, tr, ps*sp);
        populations.push_back(rf);
    }
}

RFG::~RFG() {
    // Virtual destructor implementation
    populations.clear();
}

ResultMH RFG::optimize(Problem * problem, int maxevals) {

    double groupFraction = 0.5;
    double finalFraction = 1-groupFraction;

    for(int i = 0; i < NumOfGraphs; ++i){

        populations.at(i).optimize(problem, ((maxevals*groupFraction)/NumOfGraphs),true);
    }

    Population pop(100);
    pop.populate(problem);

    double individualPortion = ceil(100/NumOfGraphs);

    for(int i = 0; i < NumOfGraphs; ++i){
        vector<int> index = populations.at(i).pop.getSortedIndex();

        for(int j = 0; j < individualPortion; ++j){
            if(i * individualPortion + j < 100){
                pop.insert_sol(i * individualPortion + j, populations.at(i).pop.getSolution(index.at(j)), populations.at(i).pop.getFitness(index.at(j)));
            }
        }
    }
        //cerr << endl << finalFraction << endl;

    
    pop.generate_graph(Population::graphTypes::RANDOM_GROUPS);

    // for(int i = 0; i < pop.getPopulationSize();++i){
    //     cout << pop.getSolution(i) << ", fitness: " << pop.getFitness(i) <<endl;
    // }
    
    RF joinedRF(pop, problem);
    ResultMH res = joinedRF.optimize(problem, maxevals*finalFraction,false);
    
    return res;
}