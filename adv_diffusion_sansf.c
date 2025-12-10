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
int Nt = 10000;
double V=0.1;
double D=0.01;

int main()
{
    double start, end, elapsed;
    int nthreads;

    // Initialisation des tableaux
    double *x = (double*)malloc((N+1)*sizeof(double));
    double *f0 = (double*)malloc((N+1)*sizeof(double));
    double *F_tab = (double*)malloc((N+1)*sizeof(double));

    double dx = 2.0 / (double)(N);

    double dt_diff= 0.5 * (dx*dx) / D;
    double dt_conv= dx / V;

    double dt = fmin(dt_diff, dt_conv);
    
    // Précalcul des coefficients
    double coeff_V = -V / dx;
    double coeff_D = D / (dx * dx);

    start = omp_get_wtime();
    
    #pragma omp parallel shared(f0,F_tab,x) firstprivate(dt,dx,N,coeff_V,coeff_D)
    {
        nthreads = omp_get_num_threads();
        
        // Initialisation de x
        #pragma omp for schedule(static) nowait
        for (int i=0;i<=N;i++)
        {
            x[i] = -1.0 + i * dx;
        }
        
        #pragma omp barrier 

        // Initialisation de f0
        #pragma omp for schedule(static) nowait
        for (int i=0; i<=N;i++)
        {
            f0[i]=2.0+cos(PI*x[i]);
        }

        // Boucle temporelle
        for (int it=0; it<Nt; it++)
        {
            // SMB
            #pragma omp for schedule(static) nowait
            for (int i=1;i<N;i++)
            {
                double df = ( f0[i] - f0[i-1]);
                double ddf = ( f0[i+1] - (2.0) * f0[i] + f0[i-1]);
                F_tab[i] = coeff_V * df + coeff_D * ddf;
            }
            
            #pragma omp single nowait
            {
                F_tab[0] = coeff_V * (f0[0] - f0[N]) + coeff_D * (f0[1] - 2.0 * f0[0] + f0[N]);
                F_tab[N] = coeff_V * (f0[N] - f0[N-1]) + coeff_D * (f0[0] - 2.0 * f0[N] + f0[N-1]);
            }
            
            #pragma omp barrier

            // Integre
            #pragma omp for schedule(static) nowait
            for (int i=0;i<=N;i++)
            {
                f0[i] = f0[i] + dt * F_tab[i];
            }
            
            #pragma omp barrier
        }
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