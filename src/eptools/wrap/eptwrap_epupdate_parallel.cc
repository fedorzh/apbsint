/* -------------------------------------------------------------------
 * EPTWRAP_EPUPDATE_PARALLEL
 *
 * Local EP updates (in parallel) for all potentials t_j(s_j) of
 * potential manager.
 *
 * Potential manager defined by POTIDS, NUMPOT, PARVEC, PARSHRD,
 * ANNOBJ. See 'PotManagerFactory' comments for full details.
 * NOTE: This is an internal transfer format, generated by Matlab
 * code.
 * If UPDIND is given, we only update on potentials j in UPDIND. In
 * this case, CMU, CRHO, RSTAT, ALPHA, NU, LOGZ are of the same
 * length as UPDIND.
 *
 * Input:
 * - POTIDS:  Potential manager representation [int32 array]
 * - NUMPOT:  " [int array]
 * - PARVEC:  " [double array]
 * - PARSHRD: " [int32 array]
 * - ANNOBJ:  " [void* array]
 * - CMU:     Vector cavity means
 * - CRHO:    Vector cavity variances
 * - UPDIND:  S.a. Optional [int32 array]
 *
 * Return:
 * - RSTAT:   Vector of return stati (1: Success, 0: Failure)
 *            [int32 array]
 * - ALPHA:   Vector of alpha values
 * - NU:      Vector of nu values
 * - LOGZ:    Vector of log Z values (optional)
 * -------------------------------------------------------------------
 * Matlab MEX Function
 * Author: Matthias Seeger
 * ------------------------------------------------------------------- */

#include "src/main.h"
#include "src/eptools/wrap/eptools_helper.h"
#include "src/eptools/wrap/eptwrap_epupdate_parallel.h"
#include "src/eptools/potentials/PotentialManager.h"

void eptwrap_epupdate_parallel(int ain,int aout,W_IARRAY(potids),
			       W_IARRAY(numpot),W_DARRAY(parvec),
			       W_IARRAY(parshrd),W_ARRAY(annobj,void*),
			       W_DARRAY(cmu),W_DARRAY(crho),W_IARRAY(updind),
			       W_IARRAY(rstat),W_DARRAY(alpha),W_DARRAY(nu),
			       W_DARRAY(logz),W_ERRORARGS)
{
  int i,j,totsz;
  double temp;
  Handle<PotentialManager> potMan;

  try {
    /* Read arguments */
    if (ain<7 || ain>8)
      W_RETERROR(2,"Wrong number of input arguments");
    if (aout<3 || aout>4)
      W_RETERROR(2,"Wrong number of return arguments");
    /* Create potential manager */
    createPotentialManager(W_ARR(potids),W_ARR(numpot),W_ARR(parvec),
			   W_ARR(parshrd),W_ARR(annobj),potMan,W_ERRARGS);
    totsz=ncmu;
    W_CHKSIZE(crho,totsz,"CRHO");
    if (ain>7) {
      if (updind==0)
	W_RETERROR(2,"UPDIND missing");
      W_CHKSIZE(updind,totsz,"UPDIND");
      Interval<int> ivM(0,potMan->size()-1,IntVal::ivClosed,IntVal::ivClosed);
      if (ivM.check(updind,nupdind)!=0)
	W_RETERROR(1,"UPDIND: Entries out of range");
    } else {
      updind=0;
      if (potMan->size()!=totsz)
	W_RETERROR(1,"CMU, potential manager: Different sizes");
    }
    /* Return arguments */
    W_CHKSIZE(rstat,totsz,"RSTAT");
    W_CHKSIZE(alpha,totsz,"ALPHA");
    W_CHKSIZE(nu,totsz,"NU");
    if (aout>3)
      W_CHKSIZE(logz,totsz,"LOGZ");
    else
      logz=0;

    /* Main loop over all potentials */
    for (i=0; i<totsz; i++) {
      j=(updind==0)?i:updind[i];
      rstat[i] =
	potMan->getPot(j).compMoments(cmu[i],crho[i],alpha[i],nu[i],&temp);
      if (rstat[i] && aout>3)
	logz[i]=temp;
    }
    W_RETOK;
  } catch (StandardException ex) {
    W_RETERROR_ARGS(1,"Caught LHOTSE exception: %s",ex.msg());
  } catch (...) {
    W_RETERROR(1,"Caught unspecified exception");
  }
}
