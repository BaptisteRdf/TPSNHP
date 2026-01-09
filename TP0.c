#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

const double PI = 3.14159265358979323846;
int N = 100000;
double V = 0.1;
double D = 0.01;

int main()
{
    double start, end, elapsed;
    
    // 1. Correction de l'allocation : N+1 éléments pour aller de 0 à N inclus
    double *x = (double*)malloc((N + 1) * sizeof(double));
    double *f_in = (double*)malloc((N + 1) * sizeof(double));
    double *f_out = (double*)malloc((N + 1) * sizeof(double));

    double dx = 2.0 / (double)N;
    double inv_dx = 1.0 / dx;       // Pré-calcul pour éviter les divisions
    double inv_dx2 = 1.0 / (dx*dx);

    double dt_diff = 0.5 * (dx * dx) / D;
    double dt_conv = dx / V;
    double dt = fmin(dt_diff, dt_conv);

    // Pré-calcul des coefficients constants pour la mise à jour
    double c1 = -V * inv_dx * dt;        // Terme advection
    double c2 = D * inv_dx2 * dt;        // Terme diffusion

    int Nt = 10000;
    int nthreads;

    start = omp_get_wtime();

    // 2. Région parallèle UNIQUE englobant tout le calcul
    #pragma omp parallel
    {
        #pragma omp master
        nthreads = omp_get_num_threads();

        // Initialisation "First Touch" (pour optimiser l'accès mémoire NUMA)
        #pragma omp for schedule(static) nowait
        for (int i = 0; i <= N; i++) {
            x[i] = -1.0 + i * dx;
            f_in[i] = 2.0 + cos(PI * x[i]);
            // On initialise f_out pour éviter des valeurs NaN aux bords si on ne les touche pas tout de suite
            f_out[i] = f_in[i]; 
        }
        // Barrière implicite ici à la fin du premier 'omp for' si on n'avait pas mis nowait. 
        // Mais ici, il faut attendre que l'init soit finie avant de lancer le temps.
        #pragma omp barrier 

        // Boucle de temps
        for (int it = 0; it < Nt; it++)
        {
            // 3. Fusion des boucles (SMB + Integre) et suppression de F_tab
            // Calcul des points intérieurs
            #pragma omp for schedule(static)
            for (int i = 1; i < N; i++)
            {
                // Calcul direct de f^{n+1}
                double df = f_in[i] - f_in[i-1];
                double ddf = f_in[i+1] - 2.0 * f_in[i] + f_in[i-1];
                
                // Formule: f_new = f_old + dt * (termes)
                // On a factorisé dt dans c1 et c2
                f_out[i] = f_in[i] + (c1 * df + c2 * ddf);
            }

            // Gestion des conditions aux limites par un seul thread (le thread maître par exemple)
            // Ou alors par les threads qui possèdent i=0 et i=N, mais 'single' est plus simple à lire.
            #pragma omp single
            {
                // Limite gauche (i=0) - Utilisation des indices périodiques simulés comme dans votre code original
                double df_0 = f_in[0] - f_in[N];
                double ddf_0 = f_in[1] - 2.0 * f_in[0] + f_in[N];
                f_out[0] = f_in[0] + (c1 * df_0 + c2 * ddf_0);

                // Limite droite (i=N)
                double df_N = f_in[N-1] - f_in[N-2]; // Attention: votre code original utilisait N-1 et N-2 pour df à N ? 
                // Vérifions votre code original pour N : coeff_V * (f0[N-1] - f0[N-2]) ...
                // C'est étrange pour un schéma décentré (habituellement f[N]-f[N-1]), mais je garde votre logique mathématique.
                double ddf_N = f_in[0] - 2.0 * f_in[N] + f_in[N-1];
                f_out[N] = f_in[N] + (c1 * df_N + c2 * ddf_N);
            }
            // Barrière implicite à la fin du 'single' ? Non, 'single' a une barrière implicite à la fin sauf si nowait.
            // Cependant, les threads du 'for' doivent attendre que les bords soient finis avant d'échanger.
            
            // Echange des pointeurs (Double Buffering)
            #pragma omp single
            {
                double *temp = f_in;
                f_in = f_out;
                f_out = temp;
            }
            // Barrière implicite ici assurant que tous les threads voient les nouveaux pointeurs avant l'itération suivante.
        }
    } // Fin de la région parallèle

    end = omp_get_wtime();
    elapsed = end - start;

    printf("%.7f secondes avec %d threads.\n", elapsed, nthreads);

    free(x);
    free(f_in);
    free(f_out);

    return 0;
}