#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef _OPENMP_
#include <omp.h>
#else
/* fallback minimal pour compilation sans OpenMP */
static double omp_get_wtime(void) { return (double)clock() / CLOCKS_PER_SEC; }
static int omp_get_num_threads(void) { return 1; }
static int omp_get_max_threads(void) { return 1; }
#endif

int main()
{
    double start, end, elapsed;
    start = omp_get_wtime();


    int N = 100000;
    int coeur = 0;
    double *tab_ini = (double*)malloc(N*sizeof(double));
    double *tab_fin = (double*)malloc(N*sizeof(double));
    #pragma omp parallel shared(tab_fin, tab_ini)
    {
        /* Récupérer le nombre de threads une seule fois */
        #pragma omp single
        coeur = omp_get_num_threads();

        #pragma omp for schedule(static)
        for (int i=0;i<N;i++)
        {
            tab_ini[i]=2.0*(double)i;
        }

        #pragma omp for schedule(static)
        for (int n=1;n<N-1;n++)
        {
            tab_fin[n]=0.25*(tab_ini[n-1]+tab_ini[n+1])+0.5*tab_ini[n];
        }
    }

    // Calcul de la première et dernière valeur

    tab_fin[0]=0.666667*tab_ini[0]+0.333333*tab_ini[1];
    tab_fin[N-1]=0.666667*tab_ini[N-1]+0.333333*tab_ini[N-2];


    end = omp_get_wtime();
    elapsed = end - start;

    for (int i=0;i<N;i++)
    {
        printf("tab_fin[%d]=%.2f \n",i,tab_fin[i]);
    }
    printf("%.7f secondes entre start et end.\n", elapsed);
    printf("Coeur : %d\n",coeur);
    /* libération de la mémoire allouée */
    free(tab_ini);
    free(tab_fin);
    return 0;
}