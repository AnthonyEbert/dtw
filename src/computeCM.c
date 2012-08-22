/* 
 * Compute global cost matrix - companion
 * to the dtw R package
 * (c) Toni Giorgino  2007-2012
 * Distributed under GPL-2 with NO WARRANTY.
 *
 * $Id: computeCM.c 268 2012-08-12 15:01:18Z tonig $
 *  
 */


#include <stdlib.h>
#include <stdio.h>
#include <math.h>


#ifdef TEST_UNIT
// Define R-like functions - a bad idea
     #define R_alloc(n,size) malloc((n)*(size))
     #define my_R_free(ptr) free((ptr))
     #define error(...) { fprintf (stderr, __VA_ARGS__); exit(-1); }
#else
     #include <R.h>
     #define my_R_free(ptr) 
#endif



#include "computeCM.h"

#ifndef NAN
#error "This code requires native IEEE NAN support. Possible solutions: 1) verify you are using gcc with -std=gnu99; 2) use the fallback interpreted DTW version (should happen automatically); 3) ask the author"
#endif


/* undo R indexing */
#define EP(ii,jj) ((jj)*nsteps+(ii))
#define EM(ii,jj) ((jj)*n+(ii))

#define CLEARCLIST { \
  for(int z=0; z<npats; z++) \
    clist[z]=NAN; }


/* 
 *  Compute cumulative cost matrix: replaces kernel in globalCostMatrix.R
 */


/* For now, this code is also valid outside R, as a test unit (the
   TEST_UNIT will be defined). This means that we have to refrain to
   use R-specific functions, such as R_malloc, or conditionally
   provide replacements when TEST_UNIT is defined */

/* R matrix fastest index is row */

void computeCM(			/* IN */
	       int *s,		/* mtrx dimensions */
	       int *wm,		/* windowing matrix */
	       double *lm,	/* local cost mtrx */
	       int *nstepsp,	/* no of steps in stepPattern */
	       double *dir,	/* stepPattern description */
				/* OUT */
	       double *cm,	/* cost matrix */
	       int *sm		/* direction mtrx */
				) {

  /* recover matrix dim */
  int n=s[0],m=s[1];		/* query,template as usual*/
  int nsteps=*nstepsp;


  /* copy steppattern description to ints,
     so we'll do indexing arithmetic on ints 
  */
  int *pn,*di,*dj;
  double *sc;

  pn=(int*) R_alloc(nsteps,sizeof(int)); /* pattern id */
  di=(int*) R_alloc(nsteps,sizeof(int)); /* delta i */
  dj=(int*) R_alloc(nsteps,sizeof(int)); /* delta j */
  sc=(double*) R_alloc(nsteps,sizeof(double)); /* step cost */

  for(int i=0; i<nsteps; i++) {
    pn[i]=dir[EP(i,0)];
    di[i]=dir[EP(i,1)];
    dj[i]=dir[EP(i,2)];
    sc[i]=dir[EP(i,3)];

    if(pn[i]<0 || pn[i]>=nsteps) {
      error("Error on pattern row %d, pattern number %d out of bounds\n",
	      i,pn[i]);
    }
  }

  /* assuming pattern ids are in ascending order */
  int npats=pn[nsteps-1];



  /* prepare a cost list per pattern */
  double *clist=(double*)
    R_alloc(npats,sizeof(double));


  /* we do not initialize the seed - the caller is supposed
     to do so
     cm[0]=lm[0];
   */


  /* lets go */
  for(int j=0; j<m; j++) {
    for(int i=0; i<n; i++) {

      /* out of window? */
      if(!wm[EM(i,j)])
	continue;

      /* already initialized? */
      if(!isnan(cm[EM(i,j)]))
	  continue;

      CLEARCLIST;
      for(int s=0; s<nsteps; s++) {
	int p=pn[s]-1;		/* indexing C-way */

	int ii=i-di[s];
	int jj=j-dj[s];
	if(ii>=0 && jj>=0) {	/* address ok? C convention */
	  double cc=sc[s];
	  if(cc==-1.0) {
	    clist[p]=cm[EM(ii,jj)];
	  } else {		/* we rely on NAN to propagate */
	    clist[p] += cc*lm[EM(ii,jj)];
	  }
	}
      }

      int minc=argmin(clist,npats);
      if(minc>-1) {
	cm[EM(i,j)]=clist[minc];
	sm[EM(i,j)]=minc+1;	/* convert to 1-based  */
      }
    }
  }


  /* Memory alloc'd by R_alloc is automatically freed */
  my_R_free(clist);
  my_R_free(sc);
  my_R_free(di);
  my_R_free(dj);
  my_R_free(pn);



}



/* return the arg min, ignoring NANs,
   -1 if all NANs */
int argmin(double *list, int n) {
  int ii=-1;
  double vv=INFINITY;
  for(int i=0; i<n; i++) {
    if(!isnan(list[i]) && list[i]<vv) {
      ii=i;
      vv=list[i];
    }
  }
  return ii;
}



/*  Unit test - made for debuggers
 ***********************************************************
*/


#ifdef TEST_UNIT

/* test main  equivalent to the following
   mylm<-outer(1:10,1:10)
   globalCostNative(mylm)->myg2
*/


#define TS 10
#define TSS (TS*TS)
int main(int argc,char **argv) {
  int ts[]={TS,TS};
  int *twm;
  double *tlm;
  int tnstepsp[]={6};
  double tdir[]={1, 1, 2, 2, 3, 3, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0,-1, 1,-1, 1,-1, 1};
  double *tcm;
  int *tsm;

  int i,j;

  twm=malloc(TSS*sizeof(int));
   for( i=0;i<TSS;i++)
    twm[i]=1;

  tlm=malloc(TSS*sizeof(double));
  for( i=0;i<TS;i++)
    for( j=0;j<TS;j++)
      tlm[i*TS+j]=(i+1)*(j+1);


  tcm=malloc(TSS*sizeof(double));
  for( i=0;i<TS;i++)
    for( j=0;j<TS;j++)
      tcm[i*TS+j]=NAN;
  tcm[0]=tlm[0];

  tsm=malloc(TSS*sizeof(int));


  double r=-2;

  tm_print(ts,tlm,&r);

  /* pretend we'r R */
  computeCM(ts,twm,tlm,tnstepsp,
	    tdir,tcm,tsm);

  tm_print(ts,tcm,&r);

  free(twm);
  free(tlm);
  free(tcm);
  free(tsm);

}



 
/*
 * Printout a matrix. 
 * int *s: s[0] - no. of rows
 *         s[1] - no. of columns
 * double *mm: matrix to print
 * double *r: return value
 */

void tm_print(int *s, double *mm, double *r) {
  int i,j;
  int n=s[0],m=s[1];
  FILE *f=stdout;

  for(i=0;i<n;i++) {
    for(j=0;j<m;j++) {
      double val=mm[j*n+i];
      if(isnan(val)) {
	//	printf("NAN %d %d\n",i,j);
      }
      fprintf(f,"[%2d,%2d] = %4.2lf    ",i,j,val);
    }
    fprintf(f,"\n");
  }
  *r=-1;
  // fclose(f);
  printf("** tm dump end **\n");
}


 #endif
