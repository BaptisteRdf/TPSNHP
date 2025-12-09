#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifndef _OPENMP
#include <omp.h>
#else
/* fallback minimal pour compilation sans OpenMP */
static double omp_get_wtime(void) { return (double)clock() / CLOCKS_PER_SEC; }
static int omp_get_num_threads(void) { return 1; }
static int omp_get_max_threads(void) { return 1; }
#endif
       
double f(double x)
{
   return 4.0/(1.0+x*x);
}

int main()

{
    double start, end, elapsed;
    start = omp_get_wtime();
    
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
        /* Récupère le nombre de threads de façon sûre */
        #pragma omp single
        coeur = omp_get_num_threads();

        #pragma omp for schedule(static)
        for (int i = 1; i < N; i++)
        {
            psomme += f((double)i * h);
        }

        #pragma omp critical
        {
            somme += psomme;
        }
    }
    somme=h*(somme+0.5*(f0+fn));
    printf("La valeur de l'intégrale est %.8f \n",somme);
    printf("Coeur : %d\n",coeur);
    
    end = omp_get_wtime();
    elapsed = end - start; /* secondes */
    printf("%.7f secondes entre start et end.\n", elapsed);
    
    return 0;
}
