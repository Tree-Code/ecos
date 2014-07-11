/*
 * ECOS - Embedded Conic Solver.
 * Copyright (C) 2012-14 Alexander Domahidi [domahidi@control.ee.ethz.ch],
 * Automatic Control Laboratory, ETH Zurich.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef __ECOS_H__
#define __ECOS_H__

#include "glblopts.h"
#include "spla.h"
#include "cone.h"
#include "kkt.h"

#if PROFILING > 0
#include "timer.h"
#endif


/* ECOS VERSION NUMBER - FORMAT: X.Y.Z --------------------------------- */
#define ECOS_VERSION ("1.0.4")


/* DEFAULT SOLVER PARAMETERS AND SETTINGS STRUCT ----------------------- */
#define MAXIT      (30)          /* maximum number of iterations         */
#define FEASTOL    (1E-7)        /* primal/dual infeasibility tolerance  */
#define ABSTOL     (1E-7)        /* absolute tolerance on duality gap    */
#define RELTOL     (1E-7)        /* relative tolerance on duality gap    */
#define FTOL_INACC (1E-4)        /* inaccurate solution feasibility tol. */
#define ATOL_INACC (1E-5)        /* inaccurate solution absolute tol.    */
#define RTOL_INACC (1E-5)        /* inaccurate solution relative tol.    */
#define GAMMA      (0.99)        /* scaling the final step length        */
#define STATICREG  (1)           /* static regularization: 0:off, 1:on   */
#define DELTASTAT  (5E-8)        /* regularization parameter             */
#define DELTA      (1E-7)        /* dyn. regularization parameter        */
#define EPS        (1E-14)  /* dyn. regularization threshold (do not 0!) */
#define VERBOSE    (1)           /* bool for verbosity; PRINTLEVEL < 3   */
#define NITREF     (3)       	 /* number of iterative refinement steps */
#define IRERRFACT  (2)           /* factor by which IR should reduce err */
#define LINSYSACC  (1E-14)       /* rel. accuracy of search direction    */
#define SIGMAMIN   (0.0001)      /* always do some centering             */
#define SIGMAMAX   (0.9999)      /* never fully center                   */
#define STEPMIN    (0.0001)      /* smallest step that we do take        */
#define STEPMAX    (0.9999) /* largest step allowed, also in affine dir. */
#define SAFEGUARD  (500)         /* Maximum increase in PRES before
                                                ECOS_NUMERICS is thrown. */

/* EQUILIBRATION METHOD ------------------------------------------------ */
#define EQUILIBRATE (1)     /* use equlibration of data matrices? >0: yes */
#define EQUIL_ITERS (3)         /* number of equilibration iterations  */
#define RUIZ_EQUIL      /* define algorithm to use - if both are ... */
/*#define ALTERNATING_EQUIL*/ /* ... commented out no equlibration is used */


/* EXITCODES ----------------------------------------------------------- */
#define ECOS_OPTIMAL  (0)   /* Problem solved to optimality              */
#define ECOS_PINF     (1)   /* Found certificate of primal infeasibility */
#define ECOS_DINF     (2)   /* Found certificate of dual infeasibility   */
#define ECOS_INACC_OFFSET (10)  /* Offset exitflag at inaccurate results */
#define ECOS_MAXIT    (-1)  /* Maximum number of iterations reached      */
#define ECOS_NUMERICS (-2)  /* Search direction unreliable               */
#define ECOS_OUTCONE  (-3)  /* s or z got outside the cone, numerics?    */
#define ECOS_FATAL    (-7)  /* Unknown problem in solver                 */


#ifdef __cplusplus
extern "C" {
#endif

/* SETTINGS STRUCT ----------------------------------------------------- */
typedef struct settings{
	pfloat gamma;                /* scaling the final step length        */
	pfloat delta;                /* regularization parameter             */
    pfloat eps;                  /* regularization threshold             */
	pfloat feastol;              /* primal/dual infeasibility tolerance  */
	pfloat abstol;               /* absolute tolerance on duality gap    */
	pfloat reltol;               /* relative tolerance on duality gap    */
    pfloat feastol_inacc; /* primal/dual infeasibility relaxed tolerance */
	pfloat abstol_inacc;  /* absolute relaxed tolerance on duality gap   */
	pfloat reltol_inacc;  /* relative relaxed tolerance on duality gap   */
	idxint nitref;				 /* number of iterative refinement steps */
	idxint maxit;                /* maximum number of iterations         */
    idxint verbose;              /* verbosity bool for PRINTLEVEL < 3    */
} settings;


/* INFO STRUCT --------------------------------------------------------- */
typedef struct stats{
	pfloat pcost;
	pfloat dcost;
    pfloat pres;
    pfloat dres;
    pfloat pinf;
    pfloat dinf;
	pfloat pinfres;
    pfloat dinfres;
    pfloat gap;
    pfloat relgap;
	pfloat sigma;
    pfloat mu;
    pfloat step;
    pfloat step_aff;
	pfloat kapovert;
	idxint iter;
    idxint nitref1;
    idxint nitref2;
    idxint nitref3;
#if PROFILING > 0
	pfloat tsetup;
	pfloat tsolve;
#endif
#if PROFILING > 1
	pfloat tfactor;
	pfloat tkktsolve;
	pfloat torder;
	pfloat tkktcreate;
	pfloat ttranspose;
	pfloat tperm;
    pfloat tfactor_t1;
    pfloat tfactor_t2;
#endif

} stats;


/* ALL DATA NEEDED BY SOLVER ------------------------------------------- */
typedef struct pwork{
	/* dimensions */
	idxint n;	/* number of primal variables x */
	idxint m;   /* number of conically constrained variables s */
	idxint p;   /* number of equality constraints */
    idxint D;   /* degree of the cone */

    /* variables */
    pfloat* x;  /* primal variables                    */
    pfloat* y;  /* multipliers for equality constaints */
    pfloat* z;  /* multipliers for conic inequalities  */
    pfloat* s;  /* slacks for conic inequalities       */
	pfloat* lambda; /* scaled variable                 */
	pfloat kap; /* kappa (homogeneous embedding)       */
	pfloat tau; /* tau (homogeneous embedding)         */

    /* best iterate seen so far */
    /* variables */
    pfloat* best_x;  /* primal variables                    */
    pfloat* best_y;  /* multipliers for equality constaints */
    pfloat* best_z;  /* multipliers for conic inequalities  */
    pfloat* best_s;  /* slacks for conic inequalities       */
    pfloat best_kap; /* kappa (homogeneous embedding)       */
	pfloat best_tau; /* tau (homogeneous embedding)         */
    pfloat best_cx;
    pfloat best_by;
    pfloat best_hz;
    stats* best_info; /* info of best iterate               */

	/* temporary stuff holding search direction etc. */
    pfloat* dsaff;
    pfloat* dzaff;
	pfloat* W_times_dzaff;
	pfloat* dsaff_by_W;
    pfloat* saff;
    pfloat* zaff;

    /* cone */
    cone* C;

    /* problem data */
    spmat* A;  spmat* G;  pfloat* c;  pfloat* b;  pfloat* h;

#if defined EQUILIBRATE && EQUILIBRATE > 0
    /* equilibration vector */
    pfloat *xequil;
    pfloat *Aequil;
    pfloat *Gequil;
#endif

	/* scalings of problem data */
	pfloat resx0;  pfloat resy0;  pfloat resz0;

	/* residuals */
	pfloat *rx;   pfloat *ry;   pfloat *rz;   pfloat rt;
	pfloat hresx;  pfloat hresy;  pfloat hresz;

	/* temporary storage */
	pfloat cx;  pfloat by;  pfloat hz;  pfloat sz;

	/* KKT System */
	kkt* KKT;

	/* info struct */
    stats* info;

	/* settings struct */
	settings* stgs;

} pwork;


/* SOME USEFUL MACROS -------------------------------------------------- */
#define MAX(X,Y)  ((X) < (Y) ? (Y) : (X))  /* maximum of 2 expressions   */
/* safe division x/y where y is assumed to be positive! */
#define SAFEDIV_POS(X,Y)  ( (Y) < EPS ? ((X)/EPS) : (X)/(Y) )


/* METHODS */

/* set up work space */
/* could be done by codegen */
pwork* ECOS_setup(idxint n, idxint m, idxint p, idxint l, idxint ncones, idxint* q,
                   pfloat* Gpr, idxint* Gjc, idxint* Gir,
                   pfloat* Apr, idxint* Ajc, idxint* Air,
                   pfloat* c, pfloat* h, pfloat* b);


/* solve */
idxint ECOS_solve(pwork* w);

/**
 * Cleanup: free memory (not used for embedded solvers, only standalone)
 *
 * Use the second argument to give the number of variables to NOT free.
 * This is useful if you want to use the result of the optimization without
 * copying over the arrays. One use case is the MEX interface, where we
 * do not want to free x,y,s,z (depending on the number of LHS).
 */
void ECOS_cleanup(pwork* w, idxint keepvars);


/**
 * Version: returns the current version number
 * Use a character array of length 7 to obtain the version number
 * in the format
 *      x.y.zzz
 * where x is the major, y the minor and zzz the build number
 */
const char* ECOS_ver(void);

#ifdef __cplusplus
}
#endif

#endif
