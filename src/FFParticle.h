/*******************************************************************************
GPU OPTIMIZED MONTE CARLO (GOMC) 2.40
Copyright (C) 2018  GOMC Group
A copy of the GNU General Public License can be found in the COPYRIGHT.txt
along with this program, also can be found at <http://www.gnu.org/licenses/>.
********************************************************************************/
#ifndef FF_PARTICLE_H
#define FF_PARTICLE_H

#include "EnsemblePreprocessor.h" //For "MIE_INT_ONLY" preprocessor.
#include "FFConst.h" //constants related to particles.
#include "Forcefield.h"
#include "BasicTypes.h" //for uint
#include "NumLib.h" //For Cb, Sq
#include "Setup.h"
#ifdef GOMC_CUDA
#include "VariablesCUDA.cuh"
#endif

// Virial and LJ potential calculation:
// U(rij) = cn * eps_ij * ( (sig_ij/rij)^n - (sig_ij/rij)^6)
//
// cn = n/(n-6) * ((n/6)^(6/(n-6)))
//
// eps_E_cn = cn * eps_ij
//                  __________const__________________
// U_lrc = density * 0.5 * 4.0 / (n-3) * cn * pi * eps_ij * sig_ij^3 *
//          ( (sig_ij/rij)^(n-3) - (n-3)/3*(sig_ij/rij)^3)
//
// Vir(r) = cn * eps_ij * n * (sig_ij/rij)^n - cn * eps_ij * 6 * (sig_ij/rij)^6
// Vir(r) = cn * eps_ij * n * repulse - cn * eps_ij * 6 * attract
// Vir(r) = cn * eps_ij * (n * repulse - 6 * attract)
// Vir(r) = cn * eps_ij * 6 * ((n/6) * repulse - attract)
//
// Vir_lrc = density * 0.5 * 4.0 * 2/3 * cn * pi * eps_ij * sig_ij^3 *
//          ( n/(n-3) * 3/2 * (sig_ij/rij)^(n-3) - 3*(sig_ij/rij)^3)

namespace ff_setup
{
class Particle;
class NBfix;
}

class Forcefield;

struct FFParticle {
public:

  FFParticle(Forcefield &ff);
  virtual ~FFParticle(void);

  virtual void Init(ff_setup::Particle const& mie,
                    ff_setup::NBfix const& nbfix);

  real GetEpsilon(const uint i, const uint j) const;
  real GetEpsilon_1_4(const uint i, const uint j) const;
  real GetSigma(const uint i, const uint j) const;
  real GetSigma_1_4(const uint i, const uint j) const;
  real GetN(const uint i, const uint j) const;
  real GetN_1_4(const uint i, const uint j) const;
  // LJ interaction functions
  virtual real CalcEn(const real distSq,
                        const uint kind1, const uint kind2) const;
  virtual real CalcVir(const real distSq,
                         const uint kind1, const uint kind2) const;
  virtual void CalcAdd_1_4(real& en, const real distSq,
                           const uint kind1, const uint kind2) const;

  // coulomb interaction functions
  virtual real CalcCoulomb(const real distSq,
                             const real qi_qj_Fact, const uint b) const;
  virtual real CalcCoulombVir(const real distSq,
                                const real qi_qj, const uint b) const;
  virtual void CalcCoulombAdd_1_4(real& en, const real distSq,
                                  const real qi_qj_Fact, const bool NB) const;

  //!Returns Energy long-range correction term for a kind pair
  virtual real EnergyLRC(const uint kind1, const uint kind2) const;
  //!Returns Energy long-range correction term for a kind pair
  virtual real VirialLRC(const uint kind1, const uint kind2) const;

  uint NumKinds() const
  {
    return count;
  }
  real GetMass(const uint kind) const
  {
    return mass[kind];
  }

#ifdef GOMC_CUDA
  VariablesCUDA *getCUDAVars()
  {
    return varCUDA;
  }
#endif

protected:
  //Find the index of the pair kind
  uint FlatIndex(const uint i, const uint j) const
  {
    return i + j * count;
  }
  //Combining sigma, epsilon, and n value for different kind
  void Blend(ff_setup::Particle const& mie, const real rCut);
  //Use NBFIX to adjust sigma, epsilon, and n value for different kind
  void AdjNBfix(ff_setup::Particle const& mie, ff_setup::NBfix const& nbfix,
                const real rCut);
  //To access rcut and other forcefield data
  const Forcefield& forcefield;

  real* mass;
  std::string *nameFirst;
  std::string *nameSec;

  //vars for LJ-LJ pairs
#ifdef MIE_INT_ONLY
  uint* n, *n_1_4;
#else
  real *n, *n_1_4;
#endif
  //For LJ eps_cn(en) --> 4eps, eps_cn_6 --> 24eps, eps_cn_n --> 48eps
  real * sigmaSq, * epsilon, * epsilon_1_4, * epsilon_cn, * epsilon_cn_6,
         * nOver6, * sigmaSq_1_4, * epsilon_cn_1_4, * epsilon_cn_6_1_4, * nOver6_1_4,
         * enCorrection, * virCorrection;

  uint count;
#ifdef GOMC_CUDA
  VariablesCUDA *varCUDA;
#endif
};


#endif /*FF_PARTICLE_H*/
