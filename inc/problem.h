#ifndef __PROBLEM_H
#define __PROBLEM_H

extern "C" {
  #include "../code/cec17.h"
}

#include "solution.h"
#include <random.hpp>
#include <utility>

// get base random alias which is auto seeded and has static API and internal
// state it is not threads secure, you can also use ::random_thread_local
using Random = effolkronium::random_static;

/**
 * Problem class to solve the given problem in metaheuristic.
 *
 * @author your_name
 * @version 1.0
 */
class Problem {

  const std::pair<tDomain,tDomain> DOMAIN_RANGE = {-100,100};

  int dim;
  std::uniform_real_distribution<double> DomainRange;

public:
  Problem(int funcid, int dim);
  /***
   * Evaluate the solution from scratch.
   * @param solution to evaluate.
   */
  tFitness fitness(tSolution &solution);
  /**
   * Create a new solution.
   */
  tSolution createSolution();
  /**
   * Return the current size of the solution.
   */
  size_t getSolutionSize() {return dim;} // Get the size of each solutionPro
  
  /** Return the range of domain of each element of the solution */
  std::pair<tDomain, tDomain> getSolutionDomainRange() {return DOMAIN_RANGE;}
};

#endif
