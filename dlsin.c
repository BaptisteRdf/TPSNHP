#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

int main()
{
	double a=3.14159/12;
	double b=1.0;
	double S=a;
	double N=32; //Degre de developpement
	
	double vsin=sin(a);
	
	for (double i=1.0;i<N-1;i+=2)
	{
		b*=-(i+1)*(i+2);
		a*=a*a;
		S+=a/b;
	}
	
	printf("La valeur est %.14f \n",S);
	printf("sin= %.14f \n",vsin);
	
	return 0;
}

