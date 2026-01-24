#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>


int main(int argc, char *argv[])
{
	
	int iCPU,NCPU; //indice du coeur, nb total coeur
	int erreur;

	MPI_Status  statut;
	erreur = MPI_Init(&argc, &argv);
	erreur = MPI_Comm_size(MPI_COMM_WORLD, &NCPU);
	erreur = MPI_Comm_rank(MPI_COMM_WORLD, &iCPU);
	srand(10000*iCPU);
	
	int N=1000000;
	int Np=N/NCPU;
	
	double *X = (double*)malloc((N/NCPU)*sizeof(double));
	double *Y = (double*)malloc((N/NCPU)*sizeof(double));
	
	
	for(int i=0;i<Np;i++)
	{
		X[i]=(double)rand()/(double)RAND_MAX;
	}
	
	if(iCPU!=0 || iCPU!=NCPU-2)
	{
		// Pour la case du proc -1 n=N/NCPU
		erreur = MPI_Send(&X[Np],1,MPI_DOUBLE,iCPU+1,51,MPI_COMM_WORLD);
		erreur = MPI_Recv(&B,1,MPI_DOUBLE,iCPU+1,84,MPI_COMM_WORLD,&statut);
		
		//Pour la case du proc +1, n=0
		erreur = MPI_Send(&X[0],1,MPI_DOUBLE,iCPU-1,84,MPI_COMM_WORLD);
		erreur = MPI_Recv(&A,1,MPI_DOUBLE,iCPU-1,51,MPI_COMM_WORLD,&statut);
	}
	
	if(iCPU==0)
	{
		erreur = MPI_Send(&X[Np],1,MPI_DOUBLE,1,66,MPI_COMM_WORLD);
		
	}

	for(int n=1, n<Np-1, n++)
	{
		double *A, *B;
		Y[n]=0.25*X[A+B]+0.5*X[n]
	}

	Y[0]=0.666667*X[0]+0.333333*X[1]; //si premier coeur et n=0
        Y[Np-1]=0.666667*X[Np-1]+0.333333*X[Np-2]; //si dernier coeur et n=Np-1
	
	
	/* libération de la mémoire allouée */
	free(X);
	free(Y);
	free(Y_Y);
	return 0;
}
