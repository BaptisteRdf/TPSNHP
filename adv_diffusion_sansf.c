#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>
#ifndef _OPENMP_
#endif

// Paramètre
const double PI=3.14159265358979323846;
int N=100000;
double V=0.1; 
double D=0.01; 


void init(double *x, double *f0, int N)
{
    #pragma omp for schedule(static)
    for (int i=0; i<N;i++)
    {
        f0[i]=2.0+cos(PI*x[i]);
    }
}



int main()
{
    double start, end, elapsed;
    int nthreads;
    start = omp_get_wtime();

    // Initialisation des tableaux
    double *x = (double*)malloc(N*sizeof(double));
    double *f0 = (double*)malloc(N*sizeof(double));
    double *F_tab = (double*)malloc(N*sizeof(double));

    double dx = 2.0 / (double)(N-1);

    double dt_diff= 0.5 * (dx*dx) / D;
    double dt_conv= dx / V;

    double dt = fmin(dt_diff, dt_conv);
    
    // Précalcul des constantes
    double coeff_V = -V / dx;
    double coeff_D = D / (dx * dx);

    // Initialisation des positions et calcul temps
    #pragma omp parallel
    {
        #pragma omp single
        nthreads = omp_get_num_threads();

        #pragma omp for schedule(static)
        for (int i=0;i<N;i++)
        {
            x[i] = i * dx;
        }
        
        // Initialisation de f0
        #pragma omp for schedule(static)
        for (int i=0; i<N;i++)
        {
            f0[i]=2.0+cos(PI*x[i]);
        }
    }

    int Nt = 10000;
    #pragma omp parallel
    {
        for (int it=0; it<Nt; it++)
        {
            #pragma omp for schedule(static)
            for (int i=1;i<N-1;i++)
            {
                double df = (f0[i] - f0[i-1]);
                double ddf = (f0[i+1] - 2.0 * f0[i] + f0[i-1]);;
                double F = coeff_V * df + coeff_D * ddf;

                // Intégration

                f0[i] += dt * F;
            }
        }
    }    end = omp_get_wtime();
    elapsed = end - start;

    printf("%.7f secondes entre start et end avec %d threads.\n", elapsed, nthreads);

    // Libération de la mémoire allouée
    free(x);
    free(f0);
    free(F_tab);

    return 0;
}