#include <problem.h>
#include <iostream>

Problem::Problem(int funcid, int dim): dim(dim) {
    cec17_init("RumorFlow", funcid, dim);
    DomainRange = std::uniform_real_distribution<double>(DOMAIN_RANGE.first,DOMAIN_RANGE.second);
}

tFitness Problem::fitness(tSolution &solution) {

    return cec17_fitness(&solution[0]);
}


tSolution Problem::createSolution(){
    tSolution sol;

    for(int i = 0; i < this->getSolutionSize(); ++i){
        sol.push_back(Random::get(DomainRange));
    }

    return sol;
}

