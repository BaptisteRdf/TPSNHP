#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>
#ifndef _OPENMP_
#endif

// Paramètre
const double PI=3.14159265358979323846;
int N=10000000;
double V=0.1;
double D=0.01;


void init(double *x, double *f0, int N)
{
    #pragma omp parallel shared(x,f0)
    {

        #pragma omp for schedule(static)
        for (int i=0; i<N;i++)
        {
            f0[i]=2.0+cos(PI*x[i]);
        }
    }   
}

void smb(double *f0, double *F_tab, double dx, int N)
{
    #pragma omp for schedule(static)
    for (int i=1;i<N;i++)
    {
        double df = ( f0[i] - f0[i-1]) / dx;
        double ddf = ( f0[i+1] - (2.0) * f0[i] + f0[i-1]) / (dx*dx);
        F_tab[i] = (-1.0)* V * df + D * ddf;
    }
}

void integre(double *f0, double *F_tab, double dt, int N)
{
    #pragma omp parallel shared(f0,F_tab)
    {
        #pragma omp for schedule(static)
        for (int i=1;i<N;i++)
        {
            f0[i] = f0[i] + dt * F_tab[i];
        }
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

    double dx = 1.0 / (double)(N-1);
    double dt = 0.0005;

    // Initialisation des positions
    #pragma omp parallel shared(x)
    {
        nthreads = omp_get_num_threads();
        #pragma omp for schedule(static)
        for (int i=0;i<N;i++)
        {
            x[i] = i * dx;
        }
    }
    

    // Initialisation de f0
    init(x, f0, N);

    int Nt = 10000000;
    for (int it=0; it<Nt; it++)
    {
        smb(f0, F_tab, dx, N);
        integre(f0, F_tab, dt, N);
    }

    end = omp_get_wtime();
    elapsed = end - start;

    printf("%.7f secondes entre start et end avec %d threads.\n", elapsed, nthreads);

    // Libération de la mémoire allouée
    free(x);
    free(f0);
    free(F_tab);

    return 0;
}