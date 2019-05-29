#include "stdafx.h"
#include "mpi.h"
#include  <time.h>
#include <iostream>
#include <fstream>


void show(double **see, int m, int q/*, string massage ="sd"*/)
{
	for (int j = 0; j < m; j++)
	{
		for (int i = 0; i < q; i++)
		{
			printf("%f ", *see[i]);
		}

		printf("\n");
	}
}


// Умножение матриц
void Matrix_X_Matrix(double **a, double **b, double **с, int m, int n, int q, int myn, int mya)
{
	for (int i = 0; i<myn; i++)
		for (int j = 0; j<m; j++)
		{
			с[i][j] = 0;
			for (int r = 0; r<n; r++)
				с[i + mya][j] += a[i][r] * b[r][j];
		}
}

int main(int argc, char *argv[])
{
	int rank, size;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	double startTime, endTime;
	startTime = MPI_Wtime();//засекаем время

	int m =100, n =130, q =100;		// Размеры матриц 
	int myN = m / size;	// Количество строк для распараллеливания на одном процессе
	if (rank == (size -1))
	{ myN += n % size; }	// Добавляем остаток

	int myA = rank * myN;
//	printf("size =%d  rank=%d  myN = %d  m =%d  myA =%d\n", size, rank, myN, m, myA);

	double **A = new double*[m];	// матрица А
	double **B = new double*[n];	// матрица В
	double **C = new double*[myN];	// матрица С
	
	// Создание двумерного массива
	for (int i = 0; i<m; i++)	
		A[i] = new double[n];	// А m,n : 100,130
	for (int i = 0; i<n; i++)	
		B[i] = new double[q];	// B n,q : 130,100
	for (int i = 0; i<myN; i++)
		C[i] = new double[q];	// C myN,q : ?,100

	// Заполняем матрицу А
	for (int i = 0; i<m; i++)
		for (int j = 0; j<n; j++)
			A[i][j] = /*1;*/4.1*((i+1) - (j+1));

	// Заполняем матрицу В
	for (int i = 0; i<n; i++)
		for (int j = 0; j<q; j++)
			B[i][j] =/*2;*/(i+1)*(j+1)*5.2;

	// подсчитываем матрису С
	Matrix_X_Matrix(A, B, C, m, n, q, myN, myA);

	int *recvCount = new int[size];	// кол-во передаваемых объектов
	MPI_Status Status;

	if (rank == 0)
	{
		int temp = 0;
		for (int i = 1; i < size; i++)
		{
			MPI_Recv(&temp, 1, MPI_INT, i, 5555, MPI_COMM_WORLD, &Status);
			recvCount[i] = temp;
		}
		recvCount[0] = myN;
	}
	else	MPI_Send(&myN, 1, MPI_INT, 0, 5555, MPI_COMM_WORLD);

	MPI_Barrier(MPI_COMM_WORLD);

	MPI_Bcast(recvCount, 1, MPI_INT, 0, MPI_COMM_WORLD);	// Пересылка на все процессы

	int *disps = new int[size];	// смещения

	if (rank == 0)
	{
		int temp = 0;
		for (int i = 1; i < size; i++)
		{
			MPI_Recv(&temp, 1, MPI_INT, i, 5555, MPI_COMM_WORLD, &Status);
			disps[i] = temp;
			//			printf("rank =%d   myN =%d\n", i, temp);
		}
		disps[0] = 0;
	}
	else	MPI_Send(&myA, 1, MPI_INT, 0, 5555, MPI_COMM_WORLD);

	MPI_Barrier(MPI_COMM_WORLD);

	MPI_Bcast(disps, 1, MPI_INT, 0, MPI_COMM_WORLD);	// Пересылка на все процессы
	double ** recvMas = new double *[m]; // принимающий буфер	//
										 //
	for (int i = 0; i < q; i++)									//
	{															//
		recvMas[i] = new double[q];								//
	}													//

	if (rank == 0)
	{
		for (int i = 0; i < myN; i++)
		{
			recvMas[i] = C[i];
		}

		for (int i = 1; i< size; i++)
			{
		//	double **buf = new double*[];
				MPI_Recv(C, recvCount[i], MPI_DOUBLE, i, 5555, MPI_COMM_WORLD, &Status);
			//	Chislo += temp;
			}
	}
	else	MPI_Send(C, myN, MPI_DOUBLE, 0, 5555, MPI_COMM_WORLD);

	if (rank == 0)
	{
		show(recvMas,m,q);
	}

	getchar();

	endTime = MPI_Wtime();//фиксируем конечное время 
	if (rank == 0)
	{
		FILE *f;
		char name[15] = "\0";
		sprintf(name, "rank %d.dat", rank);
		f = fopen(name, "w");
		fprintf(f, "rank=%d Time=%g \n", rank, endTime - startTime);

		for (int j = 0; j < m; j++)
		{
			for (int i = 0; i < q; i++)
			{
				fprintf(f, "%f ", *recvMas[i]);
			}
			fprintf(f, "\n");
		}
		fclose(f);
	}

	MPI_Finalize();
	return 0;
}