#pragma once
#include <solution.h>
#include <iostream>
#include <problem.h>
#include <stdexcept>

using namespace std;

class Population {
private:
  vector<tSolution> population;
  vector<tFitness> fitness;
  vector<vector<int>> grafo;

  const int GROUP_SIZE = 10;
  const float LOW_DENSITY = 0.05;
  const float MID_DENSITY = 0.1;
  const float HIGH_DENSITY = 0.25;

  void checkIndex(int i) const {
    if (i < 0 || i >= static_cast<int>(population.size())) {
      throw std::out_of_range("Population index out of range");
    }
  }

public:
  enum graphTypes{COMPLETE,CICLE,RANDOM_GROUPS,SYMETRICAL_GROUPS,RANDOM_LOW_DENSITY,RANDOM_MID_DENSITY,RANDOM_HIGH_DENSITY, RANDOM};

  Population(size_t size);
  void initializePop(size_t size);

  size_t getPopulationSize() const { return population.size(); }

  tFitness getFitness(int i) const {
    checkIndex(i);
    return fitness[i];
  }

  tSolution & getSolution(int i) {
    checkIndex(i);
    return population[i];
  }

  vector<int> getNeighbourIndexes(int i);
  bool findSolution(tSolution & sol);
  vector<int> getSortedIndex();
  void generate_graph(graphTypes type, float density);
  void generate_graph(graphTypes type) { generate_graph(type,0); }
  void generate_random_graph(float density);
  void populate(Problem * problem);
  void populateNeighbourhood(Problem * problem, vector<double> center, double radious);
  void set_fitness(int i, tFitness fit);
  void insert_sol(int i, tSolution & sol, tFitness fit);
  void insert_sol(int i, tSolution & sol, tFitness fit, vector<int> neightbours);
  void mutateGraph(float strength, float density);
  void mutateGraphOne(int sol, float mutationIntensity);
  void print_pop();
};
