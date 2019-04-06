/*******************************************************************************
GPU OPTIMIZED MONTE CARLO (GOMC) 2.31
Copyright (C) 2018  GOMC Group
A copy of the GNU General Public License can be found in the COPYRIGHT.txt
along with this program, also can be found at <http://www.gnu.org/licenses/>.
********************************************************************************/
#ifndef SIMULATION_H
#define SIMULATION_H

//Member vars
#include "CPUSide.h"
#include "System.h"
#include "StaticVals.h"
#include "BasicTypes.h"
#include "ReplicaExchange.h"

class Simulation
{
public:
  explicit Simulation(char const*const configFileName);
  ~Simulation();

  void RunSimulation(void);
  void RunNSteps(ulong);
  ulong getTotalSteps();
  ulong getStartStep();
  ulong getEquilSteps();
  #if ENSEMBLE == NPT || ENSEMBLE == GEMC
  double getPressure();
  double getVolume();
  double getVolume(uint i);
  #endif
  #if ENSEMBLE == GCMC
  int getNumOfParticles(uint i);
  double getChemicalPotential(uint i);
  #endif
  double getT_in_K();
  double getBeta();
  double getEpot();
  double getEpotBox(uint i);
  CPUSide* getCPUSide();
  System* getSystem();
  StaticVals* getStaticValues();
  ulong getExchangeInterval();
  int getReplExSeed();
  std::string getConfigFileName();
  std::string getMultiSimTitle();
  uint getKindOfGEMC();
  void setT_in_K(double T_in_K);
  void setBeta(double beta);
  void setCPUSide(CPUSide * cpu);
  Clock* getClock();
  ReplicaExchangeParameters* getReplExParams();
  void initReplExParams(struct config_setup::ReplicaExchangeValuesFromConf);
  void setupHierarchicalDirectoryStructure();

#ifndef NDEBUG
  void RunningCheck(const uint step);
#endif

private:
  StaticVals * staticValues;
  System *  system;
  CPUSide * cpu;
  ulong totalSteps;
  Setup set;
  std::vector<ulong> frameSteps;
  uint remarksCount;
  ulong startStep;
  double startEnergy;
  ulong absoluteTotalSteps;
  ReplicaExchangeParameters replExParams;
  std::string configFileName;
};

#endif /*SIMULATION_H*/
