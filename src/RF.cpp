#include <RF.h>
#include <util.h>

RF::RF(Population pop, Problem * problem): 
    MH(), 
    MUTATE_PTOB(0.1),                  // Default value
    MUTATE_INTENSITY(0.1),            // Default value
    SELECT_PERCENTAGE(0.2),           // Default value
    POP_SIZE(100),                     // Default value
    MAX_ROUNDS(2),                     // Default value
    SELECT_SIZE(std::min(static_cast<int>(100*0.2), 100-1)),  // Calculated from defaults
    TOURNAMENT_RATIO(0.2),             // Default value
    pop(pop) {
    
    evals = POP_SIZE;
    dist_PopSize = std::uniform_int_distribution<int>(0, static_cast<int>(POP_SIZE)-1);
}

RF::RF(Population::graphTypes graphType, Problem * problem): 
    MH(), 
    MUTATE_PTOB(0.1),                  // Default value
    MUTATE_INTENSITY(0.1),            // Default value
    SELECT_PERCENTAGE(0.2),           // Default value
    POP_SIZE(100),                     // Default value
    MAX_ROUNDS(2),                     // Default value
    SELECT_SIZE(std::min(static_cast<int>(100*0.2), 100-1)),  // Calculated from defaults
    TOURNAMENT_RATIO(0.2),            // Default value
    pop(POP_SIZE) {

    pop.populate(problem);
    pop.generate_graph(graphType);
    evals = POP_SIZE;
    dist_PopSize = std::uniform_int_distribution<int>(0, static_cast<int>(POP_SIZE)-1);
}

RF::RF(Population::graphTypes graphType, Problem * problem, float mp, double mi, float ps, int mr, double sp, float tr, float density): 
    MH(),
    MUTATE_PTOB(mp), 
    MUTATE_INTENSITY(mi), 
    POP_SIZE(ps), 
    MAX_ROUNDS(mr),
    SELECT_PERCENTAGE(sp), 
    TOURNAMENT_RATIO(tr), 
    SELECT_SIZE(std::min(static_cast<int>(ps*sp), static_cast<int>(ps)-1)),
    pop(static_cast<size_t>(ps)) {

    pop.populate(problem);
    pop.generate_graph(graphType, density);
    evals = static_cast<int>(POP_SIZE);
    dist_PopSize = std::uniform_int_distribution<int>(0, static_cast<int>(POP_SIZE)-1);
}

tFitness RF::getFitness(tSolution & sol, Problem * problem){
    
    ++evals;
    if(evals > maxevals)
        evals = maxevals;
    return problem->fitness(sol);
}

ResultMH RF::optimize(Problem * problem, int maxevals){
    return optimize(problem,maxevals,false);
}


void RF::saveDivergenceData(const string& funcName) const {
    // Create directory if it doesn't exist
    filesystem::create_directories("results_RumorFlow");
    
    string filename = "results_RumorFlow/divergence_F" + funcName + ".csv";
    ofstream outFile(filename, ios::app); // Append mode
    if (!outFile.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    // Write header only if file is empty
    outFile.seekp(0, ios::end);
    if (outFile.tellp() == 0) {
        outFile << "Evaluation,Divergence,Run\n";
    }

    // Get current run number based on file content
    int runNumber = 1;
    
    // Write data for this run
    for (const auto& [eval, div] : divergenceHistory) {
        outFile << eval << "," << div << "," << runNumber << "\n";
    }

    outFile.close();
}

ResultMH RF::optimize(Problem * problem, int maxevals, bool LocalSearch){
    LS = LocalSearch;
    this->maxevals = maxevals;

    // Clear previous divergence data
    divergenceHistory.clear();
    
    vector<int> spreaders;
    std::map<tFitness, int> solsToLocalSearch;

    int counter = 0;

    while(evals < maxevals){
        counter++;

        //SELECCIÓN
        spreaders.clear();
        for(int i = 0; i < SELECT_SIZE; ++i){
            spreaders.push_back(select());
        }

        //CRUCE
        solsToLocalSearch = propagate(spreaders,MAX_ROUNDS,problem);
        
        // //RESTART
        if(this->divergence(problem) < 20 || rejectCounter >= maxevals/5){
            rejectCounter = 0;
            vector<int> best = pop.getSortedIndex();            

            pop.mutateGraph(((double)evals/(double)maxevals)/1.5,0.8);

            tSolution bsol = tSolution(pop.getSolution(best.front()));
            tFitness bfit = pop.getFitness(best.front());
            vector<double> center;

            for(int i = 0; i < problem->getSolutionSize(); ++i){
                center.push_back(Random::get<double>(-100,100));
            }
            evals += 100;//esto puede dar fallos

            pop.populateNeighbourhood(problem,center,25);

            pop.getSolution(0) = bsol;
            pop.set_fitness(0,bfit);
        }

        //MUTACIÓN SOLUCIONES
            mutate(problem);

        // //MUTACION GRAFO
        // Store divergence data every 100 iterations
        divergenceHistory.push_back({evals, divergence(problem)});

        if(counter%100 == 0){
            // Record divergence
            
            
            double intensity = ((double)evals/(double)maxevals)/2.0;
            // // //double intensity = 0.5;
            pop.mutateGraph(intensity,0.5);

            //  vector<int> best = pop.getSortedIndex();
            //  const int MAX_LOCAL_SEARCH = 5;
            
            //  for(int i = 0; i < MAX_LOCAL_SEARCH; ++i){
            //      if(evals < maxevals)
            //          localSearch(problem,best.at(i),2000);
            //  }
        }
        //LOCAL SEARCH
    }

    int BestSolIndex = pop.getSortedIndex().front();

    return ResultMH(pop.getSolution(BestSolIndex),pop.getFitness(BestSolIndex),evals);

}

/// @brief Select operator for the EA. 
int RF::select() {
    int tournamentSize = std::min(static_cast<int>(ceil(POP_SIZE*TOURNAMENT_RATIO)), static_cast<int>(POP_SIZE)-1);
    vector<int> index(tournamentSize);

    for(int i = 0; i < tournamentSize; ++i) {
        index[i] = Random::get(dist_PopSize);
    }

    sort(index.begin(), index.end(), 
         [this](int &a, int &b) { return this->pop.getFitness(a) < this->pop.getFitness(b); });
    return index.front();
}

std::map<tFitness, int> RF::propagate(vector<int> & cromosomes, int Nrounds, Problem * problem){
    
    vector<int> spreaders;
    vector<int> newSpreaders = cromosomes;
    vector<int> neighbours;

    std::map<tFitness, int> changed;

    int counter = 0;
    
    for(int round = 0; round < Nrounds; ++round){
        spreaders = newSpreaders;
        newSpreaders.clear();
        
        sort(spreaders.begin(),spreaders.end(),[this](int &a, int &b){ return this->pop.getFitness(a) < this->pop.getFitness(b); });

        for(int cromo = 0; cromo < spreaders.size(); ++cromo){
            double percentageSpread = 1 - (double)cromo/(double)spreaders.size();
            
            percentageSpread = percentageSpread/(double)(round + 2);

            neighbours = pop.getNeighbourIndexes(spreaders.at(cromo));
            Random::shuffle(neighbours);

            int neighboursToRumor = neighbours.size()*percentageSpread;

            for(int neighbour = 0; neighbour < neighboursToRumor; neighbour++){
                if(cross(problem,spreaders.at(cromo),neighbours.at(neighbour))){
                    counter++;
                    newSpreaders.push_back(neighbours.at(neighbour));
                    changed[pop.getFitness(neighbours.at(neighbour))] = neighbours.at(neighbour);
                }
            }
        }
        
    }

    return changed;
}
    
bool RF::cross(Problem * problem, int spreader, int listener){
    bool crossed = false;

    double d = (1 - (double)evals/double(maxevals))*2;
    // // // double d = 1;
    // Avoid division by zero and handle equal fitness case
    double fitnessRatio;
    if(pop.getFitness(listener) < pop.getFitness(spreader)){
        fitnessRatio = -0.5;
    }
    else{
        fitnessRatio = 0.5;
    }

    tSolution newSol;

    for(int i = 0; i < problem->getSolutionSize(); ++ i){

        double min_ = -d* (fitnessRatio);
        double max_ = 1 + d * (fitnessRatio);
        min_ = pop.getSolution(spreader).at(i)*min_ + pop.getSolution(listener).at(i)*(1-min_);
        max_ = pop.getSolution(spreader).at(i)*max_ + pop.getSolution(listener).at(i)*(1-max_);

        if(min_ > max_){
            double aux = min_;
            min_ = max_;
            max_ = aux;
        }

        min_ = max(min_,-100.0);
        max_ = min(100.0,max_);
        if(min_ > max_){
            double aux = min_;
            min_ = max_;
            max_ = aux;
        }
        std::uniform_real_distribution<double> ratio(min_,max_ );

        double newNode = Random::get(ratio);

        newSol.push_back(newNode);

    }

    tFitness newFit;

    std::uniform_real_distribution<double> zeroOneDist(0,1);

    if(Random::get(zeroOneDist) < MUTATE_PTOB/2.0){
        newFit = mutateSol(problem,newSol,5);
    }else{
        newFit = getFitness(newSol,problem);
    }

    double error = (newFit/50);

    if(newFit <= pop.getFitness(listener) + error){
        pop.getSolution(listener) = newSol;
        pop.set_fitness(listener,newFit);
        crossed = true;
    }else{
        pop.mutateGraphOne(spreader,0.8);
        rejectCounter++;
    }


    return crossed;
}
    
void RF::mutate(Problem * problem){
    int solsToMutate = std::min(static_cast<int>(MUTATE_PTOB*POP_SIZE), static_cast<int>(POP_SIZE)-1);
    vector<int> sols(static_cast<size_t>(POP_SIZE));

    for(int i = 0; i < POP_SIZE; ++i) {
        sols[i] = i;
    }

    Random::shuffle(sols);

    for(int i = 0; i < solsToMutate; ++i) {
        tSolution newSol = pop.getSolution(sols[i]);
        tFitness newFitnes = mutateSol(problem, newSol, 5);
        
        pop.insert_sol(sols[i], newSol, newFitnes);
    }   
}

tFitness RF::mutateSol(Problem * problem, tSolution & sol, int range){

    vector<int> nodes(problem->getSolutionSize());
    int nodesToMutate = MUTATE_INTENSITY*problem->getSolutionSize();

    for(int i = 0; i < problem->getSolutionSize(); ++i){
        nodes.at(i) = i;
    }
    
    
    Random::shuffle(nodes);
    
    for(int j = 0; j < nodesToMutate; ++j){

        double max_ = min(100.0,sol.at(nodes.at(j)) + range);
        double min_ = max(-100.0,sol.at(nodes.at(j)) - range);
        std::uniform_real_distribution<double> randDist(min_,max_);
        
        sol.at(nodes.at(j)) = Random::get(randDist);
    }
    
    return getFitness(sol,problem);
}

void RF::localSearch(Problem * problem, int cromosome, int evals){
    std::uniform_real_distribution<double> randDist(-1,1);

    tFitness bestFit = pop.getFitness(cromosome);
    tSolution workingSol = this->pop.getSolution(cromosome);
    tSolution neighbourSol;

    for(int i = 0; i < evals; ++i){

        neighbourSol = workingSol;

        for(int j = 0; j < problem->getSolutionSize(); ++j){
            neighbourSol.at(j) += Random::get(randDist);

            if(neighbourSol.at(j) > 100)
                neighbourSol.at(j) = 100;
            else if( neighbourSol.at(j) < -100){
                neighbourSol.at(j) = -100;
            }
        }

        tFitness fit = this->getFitness(neighbourSol,problem);

        if(fit < bestFit){
            workingSol = neighbourSol;
            bestFit = fit;
        }
    }
    
    pop.insert_sol(cromosome,workingSol,bestFit);
    
}

double RF::divergence(Problem * prob){
    double sumOfDistances = 0.0;  // Initialize to 0

    int size = pop.getPopulationSize();

    for(int i = 0; i < size; ++i){
        for(int j = i+1; j < size; ++j){
            double sumOfVectors = 0.0;  // Initialize for each pair

            for(int s = 0; s < prob->getSolutionSize(); s++){
                sumOfVectors += pow((pop.getSolution(i).at(s) - pop.getSolution(j).at(s)), 2);
            }

            sumOfDistances += sqrt(sumOfVectors);
        }
    }

    if (size < 2) return 0.0;
    return sumOfDistances / ( (double)(size * (size - 1)) / 2.0 );
}

vector<double> RF::deviation(Problem * prob){
    vector<double> s1;
    vector<double> s2;

    for(int j = 0; j < prob->getSolutionSize()/10;++j){
        s1.push_back(0);
        s2.push_back(0);
    }

    for(int i = 0; i < pop.getPopulationSize()/10; ++i){
        for(int j = 0; j < prob->getSolutionSize();++j){
            s1.at(j) += pop.getSolution(i).at(j);
            s2.at(j) += pow(pop.getSolution(i).at(j),2);
        }
    }

    vector<double> deviation;

    for(int i = 0; i < prob->getSolutionSize()/10;++i){
        deviation.push_back(sqrt((pop.getPopulationSize()*s2.at(i) - s1.at(i)*s1.at(i))/(pop.getPopulationSize()*(pop.getPopulationSize() - 1)) ));
    }

    return deviation;
}
