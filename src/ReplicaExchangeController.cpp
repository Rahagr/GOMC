/*******************************************************************************
GPU OPTIMIZED MONTE CARLO (GOMC) 2.31
Copyright (C) 2018  GOMC Group
A copy of the GNU General Public License can be found in the COPYRIGHT.txt
along with this program, also can be found at <http://www.gnu.org/licenses/>.
********************************************************************************/
#include "ReplicaExchangeController.h"

using namespace std; 


ReplicaExchangeController::ReplicaExchangeController(vector<Simulation*>* sims){

    simsRef = sims;
    //  For now, exchangeRate == the arg for RunNSteps; which removes the necessity to check if this step is an exchange step
    //  We simply exchange after each burst
    exchangeRate= (*simsRef)[0]->getExchangeInterval();
    totalSteps = (*simsRef)[0]->getTotalSteps();
    checkerForIncreasingMontonicityOfTemp = 0;
    std::string multiSimTitle = (*simsRef)[0]->getMultiSimTitle();

    for ( int i = 0; i < (*simsRef).size(); i++){
        if (exchangeRate != (*simsRef)[i]->getExchangeInterval()){
            std::cout << "Error: Each replica must have equal exchange rate. " << (*simsRef)[i]->getConfigFileName() <<
            " differs from the others!\n";
            exit(EXIT_FAILURE);
        }
        if (multiSimTitle.compare((*simsRef)[i]->getMultiSimTitle())){
            std::cout << "Error: Each replica must have the same multiSimTitle. " << (*simsRef)[i]->getConfigFileName() <<
            " differs from the others!\n";
            exit(EXIT_FAILURE);
        }
        if (totalSteps != (*simsRef)[i]->getTotalSteps()){
            std::cout << "Error: Each replica must have number of total steps. " << (*simsRef)[i]->getConfigFileName() <<
            " differs from the others!\n";
            exit(EXIT_FAILURE);
        }
        if ( (*simsRef)[i]->getT_in_K() > checkerForIncreasingMontonicityOfTemp ){
          checkerForIncreasingMontonicityOfTemp = (*simsRef)[i]->getT_in_K();
        } else {
            std::cout << "Error: List the conf files in increasing temperature order. " << (*simsRef)[i]->getConfigFileName() <<
            " is not in order of least to greatest for temperature!\n";
            exit(EXIT_FAILURE);
        }
    }

    if (exchangeRate > 0) {
      roundedUpDivison = ((*simsRef)[0]->getTotalSteps() + exchangeRate - 1) / exchangeRate;
    } else {
      exchangeRate = totalSteps;
      roundedUpDivison = 1;
    }



}
//~ReplicaExchange();

void ReplicaExchangeController::runMultiSim(){
    for (ulong i = 0; i < roundedUpDivison; i++){
        for (int j = 0; j < (*simsRef).size(); j++){
          // Note that RunNSteps overwrites startStep before returning to the step it left off on
          (*simsRef)[j]->RunNSteps(ulong(exchangeRate));
        }
        if (exchangeRate!=totalSteps)
          for (int j = 0; j < (*simsRef).size(); j++){
            if ((*simsRef)[j]->getEquilSteps() < ((*simsRef)[j]->getStartStep() + exchangeRate)) {
              //  To alternate between swapping even replicas and repl_id+1 {0,1} {2,3} ... on even parity and 
              //  odd replicas and repl_id+1 {1,2} ... on odd parity
              parityOfSwaps = ((*simsRef)[j]->getStartStep() / exchangeRate) % 2;
              if (j % 2 == parityOfSwaps){
                if (j + 1 < (*simsRef).size()){
                  swapperForT_in_K = (*simsRef)[j]->getT_in_K(); 
                  swapperForBeta = (*simsRef)[j]->getBeta();
                  swapperForCPUSide = (*simsRef)[j]->getCPUSide();
                  (*simsRef)[j]->setT_in_K((*simsRef)[j+1]->getT_in_K());
                  (*simsRef)[j]->setBeta((*simsRef)[j+1]->getBeta());
                  (*simsRef)[j]->setCPUSide((*simsRef)[j+1]->getCPUSide());
                  (*simsRef)[j+1]->setT_in_K(swapperForT_in_K);
                  (*simsRef)[j+1]->setBeta(swapperForBeta);
                  (*simsRef)[j+1]->setCPUSide(swapperForCPUSide);
                }
              }
            }
          }
      }
}


