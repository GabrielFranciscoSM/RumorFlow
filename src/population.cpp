#include <population.h>
#include <util.h>
#include <algorithm>
#include <float.h>

Population::Population(size_t size){
    this->initializePop(size);
}

void Population::initializePop(size_t size){
    population.resize(size);
    fitness.resize(size);
    grafo.resize(size, vector<int>(0));
}

vector<int> Population::getNeighbourIndexes(int i){
    vector<int> neighbours;

    int numberOfNeightbours = grafo.at(i).size();

    for(int j = 0; j < numberOfNeightbours; ++j){
        neighbours.push_back(grafo.at(i).at(j));
    }

    return neighbours;
}


void Population::insert_sol(int i, tSolution & sol, tFitness fit){
    insert_sol(i,sol,fit, getNeighbourIndexes(i));
}

void Population::insert_sol(int i, tSolution & sol, tFitness fit, vector<int> neightbours){
    population.at(i) = sol;
    fitness.at(i) = fit;

    grafo.at(i) = neightbours;
}


void Population::set_fitness(int i, tFitness fit){
    fitness.at(i) = fit;
}

void Population::print_pop(){
    for(int i = 0; i < this->population.size(); ++i){
        std::cout << this->population.at(i) << endl;
    }
}

bool Population::findSolution(tSolution & sol){
    return !(find(this->population.begin(),this->population.end(),sol) == this->population.end());
}

vector<int> Population::getSortedIndex(){

    vector<int> index(this->getPopulationSize());

    for(int i = 0; i < this->getPopulationSize(); ++i){
        index.at(i) = i;
    }


    sort(index.begin(),index.end(),[this](int &a, int &b){ return this->getFitness(a) < this->getFitness(b); });

    return index;
}

void Population::generate_graph(graphTypes gt, float density){

    for(int i = 0; i < grafo.size(); ++i){
        grafo.at(i).clear();
    }

    switch (gt)
    {
    case graphTypes::CICLE:
        for(int i = 0; i < grafo.size(); ++i){
            grafo.at(i).push_back((i+1)%grafo.size());
        }
        break;
    
    case graphTypes::RANDOM_LOW_DENSITY:
        {
            this->generate_random_graph(LOW_DENSITY);
        }
        break;

    case graphTypes::RANDOM_MID_DENSITY:
        {
            this->generate_random_graph(MID_DENSITY);
        }
        break;

    case graphTypes::RANDOM_HIGH_DENSITY:
        {
            this->generate_random_graph(HIGH_DENSITY);
        }
        break;
    case graphTypes::RANDOM:
        {
            this->generate_random_graph(density);
        }
        break;
    
    case graphTypes::COMPLETE:

        for(int i = 0; i < grafo.size(); ++i){
            for(int j = 0; j < grafo.size(); ++j){
                if(i != j)
                    grafo.at(i).push_back(j);
            }
        }

        break;
    case graphTypes::RANDOM_GROUPS:
    {
        std::uniform_int_distribution<int> numberOfNeighbours(1,0.15*this->getPopulationSize());

        vector<int> groupConnectors;

        for(int i = 0; i < grafo.size();++i){
            std::uniform_int_distribution<int> neighbourDist((int)ceil(i/GROUP_SIZE) * GROUP_SIZE,min( (int)(this->getPopulationSize()-1),(int)ceil((i+GROUP_SIZE)/GROUP_SIZE)*GROUP_SIZE ));
            int numOfNeightbours = Random::get(numberOfNeighbours);

                for(int j = 0; j < numOfNeightbours; ++j){
                    int aux = Random::get(neighbourDist);

                    while(aux == i)
                        aux = Random::get(neighbourDist);

                    grafo.at(i).push_back(aux);
                }

                if(i%GROUP_SIZE == 0){
                    groupConnectors.push_back(i);
                }
        }
//            grafo.at(groupConnectors.at(i)).push_back(groupConnectors.at((i+1)%groupConnectors.size()));

        for(int i = 0; i < groupConnectors.size(); ++i){
            for(int j = 0; j < groupConnectors.size(); ++j){
                if(i != j)
                grafo.at(groupConnectors.at(i)).push_back(groupConnectors.at(j));
            }
        }
    }
        break;

    case graphTypes::SYMETRICAL_GROUPS:

    {
        for(int i = 0; i < grafo.size();++i){
            int startpos = i - i%GROUP_SIZE;
            int endpos = min(startpos+GROUP_SIZE,(int)grafo.size());

            for(int j = startpos; j < endpos; ++j){
                grafo.at(i).push_back(j);
            }

            if(startpos == i && i != 0){
                grafo.at(i).push_back(i-1);
            }

            if(i == 0){
                grafo.at(0).push_back(grafo.size()-1);
            }
        }
    }
        break;

    default:
        break;
    }
}

void Population::populate(Problem * problem){
    
    population.at(0) = problem->createSolution();

    fitness.at(0) = problem->fitness(population.at(0));

    for(int i = 1; i < this->getPopulationSize(); ++i){

        population.at(i) = problem->createSolution();

        fitness.at(i) = problem->fitness(population.at(i));
    }
}

void Population::populateNeighbourhood(Problem * problem, vector<double> center, double radious){
    


    tSolution aux;

    for(int i = 0; i < problem->getSolutionSize(); ++i){
        double min_ = center.at(i) - radious;
        double max_ = center.at(i) + radious;

        min_ = max(-100.0, min_);
        max_ = min(100.0,max_);
        
        std::uniform_real_distribution<double> node(min_,max_);

        aux.push_back(Random::get(node));
    }

    population.at(0) = aux;

    fitness.at(0) = problem->fitness(population.at(0));

    for(int i = 1; i < this->getPopulationSize(); ++i){

        for(int j = 0; j < problem->getSolutionSize(); ++j){
            double min_ = center.at(j) - radious;
            double max_ = center.at(j) + radious;

            min_ = max(-100.0, min_);
            max_ = min(100.0,max_);
            
            std::uniform_real_distribution<double> node(min_,max_);

            aux.at(j) = (Random::get(node));
        }

        population.at(i) = aux;

        fitness.at(i) = problem->fitness(population.at(i));
    }
}

void Population::generate_random_graph(float density){

    std::uniform_int_distribution<int> neighbourDist(0,this->getPopulationSize()-1);
    std::uniform_int_distribution<int> numberOfNeighbours(1,density*this->getPopulationSize());

    int numOfNeightbours;

    for(int i = 0; i < grafo.size(); ++i){
        numOfNeightbours = Random::get(numberOfNeighbours);

        for(int j = 0; j < numOfNeightbours; ++j){
            int aux = Random::get(neighbourDist);

            while(aux == i)
                aux = Random::get(neighbourDist);

            grafo.at(i).push_back(aux);

        }
    }
}

void Population::mutateGraph(float percentageNodes, float mutationIntensity){

    vector<int> sols;
    int densidad = 0;


    for(int i = 0; i < this->grafo.size(); ++i ){
        sols.push_back(i);
        densidad += grafo.at(i).size();
    }

    densidad /= grafo.size();

    Random::shuffle(sols);

    int nodesToMutate = this->grafo.size()* percentageNodes;
    int IntensityNodes = densidad* mutationIntensity;

    std::uniform_int_distribution<int> nodeDist(0,this->grafo.size()-1);


    for(int i = 0; i < nodesToMutate; ++i){

        if(grafo.at(sols.at(i)).size() > IntensityNodes)
            grafo.at(sols.at(i)).resize(grafo.at(sols.at(i)).size() - IntensityNodes);

        for(int j = 0; j < IntensityNodes; ++j){
            grafo.at(sols.at(i)).push_back(Random::get(nodeDist));
        }
    }
}

void Population::mutateGraphOne(int sol, float mutationIntensity){

    int densidad = 0;
    std::uniform_int_distribution<int> nodeDist(0,this->grafo.size()-1);

    for(int i = 0; i < this->grafo.size(); ++i ){
        densidad += grafo.at(i).size();
    }

    densidad /= grafo.size();
    int IntensityNodes = densidad* mutationIntensity;

    if(grafo.at(sol).size() > IntensityNodes)
        grafo.at(sol).resize(grafo.at(sol).size() - IntensityNodes);

    for(int j = 0; j < IntensityNodes; ++j){
            grafo.at(sol).push_back(Random::get(nodeDist));
    }
}

