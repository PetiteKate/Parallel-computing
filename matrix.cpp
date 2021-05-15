#include <iostream>
#include <cstdlib>
#include <ctime>
//#include "omp.h"

using namespace std;

#define n 1000

void printMatrix(int **Matrix)
{
    for(int i=0; i<n; i++)
    {
        for(int j=0; j<n; j++)
        {
            cout << Matrix[i][j] ;
        }
        cout << "\n";
    }
    cout << "\n";
}

void deleteMatrix(int **Matrix)
{
    for(int i=0; i<n; i++)
        delete [] Matrix[i];
    delete [] Matrix;
}

int main() {

    int i, j, k;
    int **A;
    int **B;
    int **C;

    A = new int *[n];
    B = new int *[n];
    C = new int *[n];

    clock_t start,  middle, finish, tstart;

    for(i=0; i<n; i++)
    {
        A[i] = new int [n];
        B[i] = new int [n];
        C[i] = new int [n];
        for(j=0; j<n; j++)
        {
            A[i][j] = rand()%2;
            B[i][j] = rand()%2;
            C[i][j] = 0;
        }
    }

    //printMatrix(A);
    //printMatrix(B);

    start = clock();

    #pragma omp parallel private(i, j, k, tstart, middle) num_threads(2)
    {
        //#pragma omp for schedule(dynamic, 10) nowait
        for(i=0; i < n; i++)
        {
            //tstart = clock();
            //#pragma omp for schedule(dynamic, 10) nowait
            for(j=0; j < n; j++)
            {
                //#pragma omp for schedule(dynamic, 10) nowait
                for (k = 0; k < n; k++)
                    C[i][j] += A[i][k] * B[k][j];
            }
            //middle = clock();
            //printf("Task %d! (%d, %d, %d). --- Th. time = %2.2f s. Cur. Time = %2.2f s. \n", i, C[i][0], C[i][1], C[i][2], (double)(middle - tstart) / CLOCKS_PER_SEC, (double)(middle - start) / CLOCKS_PER_SEC);
        }
    }
    finish = clock();

    //printMatrix(C);

    printf("Elapsed Time %2.5f seconds\n", (double)(finish - start) / CLOCKS_PER_SEC);


    deleteMatrix(A);
    deleteMatrix(B);
    deleteMatrix(C);

    //system("pause");
    return 0;
}
