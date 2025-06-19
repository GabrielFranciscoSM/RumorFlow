extern "C" {
  #include "code/cec17.h"
}

#include <iostream>
#include <problem.h>
#include <random.hpp>
#include <string>
#include <util.h>
#include <fstream>
#include <sstream>
#include <chrono>
#include <mh.h>
#include <string>

#include <population.h>

#include <RF.h>
#include <RFGroups.h>

using namespace std;

// Function to print usage instructions
void printUsage(const char* progName) {
  std::cerr << "Usage: " << progName << " [options]\n\n"
            << "Options:\n"
            << "  -t               Enable terminal output                                   \n"
            << "  -s <seed file>   Set the problem seeds to the one given in the seed file. \n"
            << "  --mp <value>     Mutation probability (0-1). Default: 0.2\n"
            << "  --mi <value>     Mutation intensity (0-2). Default: 0.75\n"
            << "  --ps <value>     Population size (50-200). Default: 100\n"
            << "  --mr <value>     Max rounds (1-5). Default: 2\n"
            << "  --sp <value>     Select percentage (0-1). Default: 0.05\n"
            << "  --tr <value>     Tournament ratio (0-1). Default: 1\n"
            << "  -h, --help       Show this help message                                 \n\n"
            << "Example:                                                                    \n"
            << "  " << progName << " -f 1 --mp 0.3 --mi 1.0 --ps 150 --mr 3 --sp 0.1 --tr 0.5\n";
}

void usualPrint(std::ostream &out, string name, tSolution bestSol, tFitness bestFitness, float meanFit, std::chrono::duration<double> meanTime, float meanEvals ){
  out << name << endl;
  out << "Best solution:    " << bestSol << endl;
  out << "Best fitness:     " << bestFitness << endl;
  out << "Mean fitness:     " << meanFit/5.0 << endl;
  out << "Computation time: " << meanTime.count()/5.0 << " seconds" << endl;
  out << "Evaluations:      " << meanEvals/5.0 << endl << endl;
}

void tablePrint(std::ostream &out, string name, tFitness bestFitness, float meanFit, std::chrono::duration<double> meanTime, float meanEvals ){
  out << 
  name << "," << 
  bestFitness << "," << 
  meanFit/5.0 << "," << 
  meanTime.count()/5.0 << "," <<
  meanEvals/5.0 << endl;  
}

int main(int argc, char *argv[]) {
    long int seed = 42;
    std::string outputMode = "";
    std::string seedsFile = "../datos/seeds.txt";
    
    // RF parameters with defaults        

    // float mutationProb = 0.1;        
    // double mutationIntensity = 0.3;  
    // float populationSize = 100;        
    // int maxRounds = 2;
    // double selectPercentage = 0.2;   
    // float tournamentRatio = 0.3;      
    // float density = 0.1;  

    // float mutationProb = 0.1;        
    // double mutationIntensity = 0.1;  
    // float populationSize = 100;        
    // int maxRounds = 2;
    // double selectPercentage = 0.2;   
    // float tournamentRatio = 0.2;      
    // float density = 0.1;       

    float mutationProb = 0.1;        
    double mutationIntensity = 0.1;  
    float populationSize = 100;        
    int maxRounds = populationSize/100 * 2;
    double selectPercentage = 0.2;   
    float tournamentRatio = 0.2;      
    float density = 0.1;   

    //Parameter input processing
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "--mp" && i + 1 < argc) {
            mutationProb = std::stof(argv[++i]);
        } else if (arg == "--mi" && i + 1 < argc) {
            mutationIntensity = std::stod(argv[++i]);
        } else if (arg == "--ps" && i + 1 < argc) {
            populationSize = std::stof(argv[++i]);
        } else if (arg == "--mr" && i + 1 < argc) {
            maxRounds = std::stoi(argv[++i]);
        } else if (arg == "--sp" && i + 1 < argc) {
            selectPercentage = std::stod(argv[++i]);
        } else if (arg == "--tr" && i + 1 < argc) {
            tournamentRatio = std::stof(argv[++i]);
        } else if (arg == "--de" && i + 1 < argc) {
            density = std::stof(argv[++i]);
        } else if (arg == "-s" && i + 1 < argc) {
            seedsFile = argv[++i];
        } else if (arg == "-t") {
            outputMode = "terminal";
        }
    }

  //Prefixed seeds for deterministic results
  std::vector<int> seeds;
  std::ifstream file(seedsFile);

  if (!file.is_open()) {
      std::cerr << "Error: Unable to open file " << seedsFile << std::endl;
      return 1; // Return empty vector on error
  }

  int num;
  while (file >> num) {
      seeds.push_back(num);
  }

  file.close();

  if(seeds.size() != 5){
    std::cerr << "Error: Wrong number of seeds given. There are "  << seeds.size() << "seeds but has to be 5" << std::endl;
    return 1;
  }
    
  int dim = 10;

  for(int i = 0; i < 5; ++i){

    Random::seed(seeds[i]);

    for(int funcId = 1; funcId <= 30; ++funcId){
      Problem prob(funcId,dim);

      //cerr <<mutationProb <<mutationIntensity << populationSize << maxRounds << selectPercentage << tournamentRatio << endl;
      
      RF rf(Population::graphTypes::RANDOM_MID_DENSITY, &prob, 
            mutationProb, mutationIntensity, populationSize, maxRounds, selectPercentage, tournamentRatio, density);
      ResultMH res = rf.optimize(&prob,dim*10000,false);

      rf.saveDivergenceData(std::to_string(funcId));
      //    RFG rf(Population::graphTypes::RANDOM_MID_DENSITY, &prob, 
      //        mutationProb, mutationIntensity, populationSize, maxRounds, selectPercentage, tournamentRatio, density);
      //  ResultMH res = rf.optimize(&prob,dim*10000);

      cout <<"Best Random[F" <<funcId <<"]: " << scientific <<cec17_error(res.fitness) << ", Evaluations: " << res.evaluations << endl;
    }

  }

  //int funcId = 4;

  return 0;
}


