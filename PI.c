#include <stdio.h>
#include <stdlib.h>
#include <time.h>
    
 
 
       
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
    double N=1000000000.0;
    double I=0.0;
    double h=(b-a)/N;
    

    double f0=f(a);
    double fn=f(b);
    
    for (double i=1.0;i<N;i++)
    {
      I+=f(i*h);
    }

    I=h*((I)+0.5*(f0+fn));
    printf("La valeur de l'intégrale est %.8f \n",I);
    
    
    end = clock(); 
    elapsed = ((double)end - (double)start) / CLOCKS_PER_SEC; /* Conversion en seconde  */
    printf("%.7f secondes entre start et end.\n", elapsed);
    
    return 0;
}
