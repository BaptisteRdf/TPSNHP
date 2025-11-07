#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifndef _OPENMP_
#include <omp.h>
#endif
       
double f(double x)
{
   return 4.0/(1.0+x*x);
}

int main()

{
    clock_t start, end;
    double elapsed;
    start = clock();
    
    double a=0.0;
    double b=1.0;
    int N=1000000000;
    double psomme=0.0;
    double h=(b-a)/(double)N;
    double somme=0.0;
    int coeur=0;
    double f0=f(a);
    double fn=f(b);
    
    #pragma omp parallel shared(somme) firstprivate(psomme) 
    {
    	coeur=omp_get_num_threads();
    	#pragma omp for schedule(static)
    	for (int i=1;i<N;i++)
    	{
      		psomme+=f((double)i*h);
    	}
	#pragma omp critical
	{
		somme+=psomme;	
    	}	
    }
    somme=h*(somme+0.5*(f0+fn));
    printf("La valeur de l'intégrale est %.8f \n",somme);
    printf("Coeur : %d\n",coeur);
    
    end = clock(); 
    elapsed = ((double)end - (double)start) / CLOCKS_PER_SEC/ (double)coeur; /* Conversion en seconde  */
    printf("%.7f secondes entre start et end.\n", elapsed);
    
    return 0;
}
