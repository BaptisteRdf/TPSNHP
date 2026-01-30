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
    


/*Pour les 3 fonctions qui vont suivre, on realise le pragma for sans ouvrir de zone parallèle pour eviter qu'à chaque pas de temps
elle ne s'ouvre 3 fois, l'ouverture de la prallelisation se fait dans le main*/
void init(double *x, double *f0, int N)
{
    #pragma omp for schedule(static)
    for (int i=0; i<=N;i++)
    {
        f0[i]=2.0+cos(PI*x[i]); // Condition initiale
    }  
}


/*Pour cette fonction, j'ai pense qu'il serait plus rapide de passer directement avec les valeurs du tableau, que de passer par la derivee
Pour se faire on part de l'equation d'origine et on met tout sous le même facteur commun 1/dx^2 soit 0.25*N^2. Le détail des coefficient se trouve 
dans le main*/

void smb(double *f0, double *F_tab, double dx, int N, double coeff_A, double coeff_B, double coeff_C)
{
    #pragma omp for schedule(static)  
    for (int i=1;i<N;i++)
    {
        F_tab[i] = coeff_A * f0[i-1] + coeff_B * f0[i] + coeff_C * f0[i+1];
    }
    /*La fonction etant periodique, on traite les bords en dehors de la boucle pour eviter des conditions if dans la boucle,
     le N+1 en N devient 0 et le N-1 en 0 devient N */
    #pragma omp single  
    {
        F_tab[0] = coeff_A * f0[N] + coeff_B * f0[0] + coeff_C * f0[1]; // Condition aux limites periodiques en 0
        F_tab[N] = coeff_A * f0[N-1] + coeff_B * f0[N] + coeff_C * f0[0]; // Condition aux limites periodiques en N
    }
}

void integre(double *f0, double *F_tab, double dt, int N)
{
    #pragma omp for schedule(static)  
    for (int i=0;i<=N;i++)
    {
        f0[i] += dt *F_tab[i];
    }
}

int main()
{
    double start, end, elapsed; // Variables pour le chrono
    int nthreads;
    

    // Initialisation des tableaux
    double *x = (double*)malloc((N+1)*sizeof(double)); // 0 a N donc N+1 elements
    double *f0 = (double*)malloc((N+1)*sizeof(double));
    double *F_tab = (double*)malloc((N+1)*sizeof(double));

    double dx = 2.0 / (double)(N);

    // Calcul du pas de temps pour la stabilite
    double dt_diff= 0.5 * (dx*dx) / D;
    double dt_conv= dx / V;

    double dt = fmin(dt_diff, dt_conv);
    
    // Precalcul des coefficients
    double coeff_A = 0.25*N*N*D + 0.25*N*N*V*dx;
    double coeff_B = 0.25*N*N*D;
    double coeff_C = 0.25*N*N*(-2)*D - 0.25*N*N*V*dx;

    start = omp_get_wtime(); // Debut du chrono
    

    #pragma omp parallel shared(f0,F_tab,x) firstprivate(dt,dx,N,Nt,coeff_A,coeff_B,coeff_C) // On ouvre la prallelisation une seule fois
    {
        nthreads = omp_get_num_threads();
        #pragma omp for schedule(static) // Initialisation du vecteur position
        for (int i=0;i<=N;i++)
        {
            x[i] = -1.0 + i * dx; // Domaine [-1,1]
        }

        // Initialisation de f0
        init(x, f0, N);

        for (int it=0; it<Nt; it++) // Pas de parallelisation ici, l'etat t+1 depend de t
        {
            smb(f0, F_tab, dx, N, coeff_A, coeff_B, coeff_C); 
            integre(f0, F_tab, dt, N);
        }
    }
    

    end = omp_get_wtime();
    elapsed = end - start;

    printf("%.7f secondes entre start et end avec %d threads.\n", elapsed, nthreads);

    // Liberation de la memoire allouee
    free(x);
    free(f0);
    free(F_tab);

    return 0;
}