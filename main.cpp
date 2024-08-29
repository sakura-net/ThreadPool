#include "ThreadPool.h"
#include<windows.h>
#include <cfloat>
#include <stdio.h>
#include<ctime>
#include <random>
#include<iostream>
#include<omp.h>
#define BLOCK_SIZE 16
using namespace CCPool;
using namespace std;
typedef struct _mat {
    int** c; int** a; int** b;
    int size;
    int i, j;
}mat;
double sequentialTestAns(int** c, int** a, int** b, int size) {

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            c[i][j] = 0;
        }
    }

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            for (int k = 0; k < size; k++) {
                c[i][j] += a[i][k] * b[k][j];
            }
        }
    }
    return 0;
}
void Block(void* n) {
    mat* m = (mat*)n;
    int size = m->size;
    int** a = m->a;
    int** b = m->b;
    int** testans = m->c;
    int i = m->i;
    int j = m->j;
    int temp[BLOCK_SIZE][BLOCK_SIZE] = { 0 };
    for (int k = 0; k < size; k += BLOCK_SIZE) {
        for (int ii = i; ii < i + BLOCK_SIZE && ii < size; ii++) {
            for (int jj = j; jj < j + BLOCK_SIZE && jj < size; jj++) {
                for (int kk = k; kk < k + BLOCK_SIZE && kk < size; kk++) {
                    temp[ii - i][jj - j] += a[ii][kk] * b[kk][jj];
                }
            }
        }
    }
    for (int ii = i; ii < i + BLOCK_SIZE && ii < size; ii++) {
        for (int jj = j; jj < j + BLOCK_SIZE && jj < size; jj++) {
            testans[ii][jj] = temp[ii - i][jj - j];
        }
    }
    delete m;
}
int TestAns(int** testans, int** a, int** b, int size) {
    ThreadPool* pool;
    pool = PoolInit(20);
    for (int i = 0; i < size; i += BLOCK_SIZE) {
        for (int j = 0; j < size; j += BLOCK_SIZE) {
            mat* as = new mat;
            as->a = a;
            as->b = b;
            as->c = testans;
            as->i = i;
            as->j = j;
            as->size = size;
            PoolAdd(pool, Block, (void*)as);
        }
    }
    PoolSync(pool);

    return 0;
}
int main() {
    // Initialize matrices c, a, and b with appropriate sizes
    int arraySize;
    cin >> arraySize;
    uniform_int_distribution<unsigned >u(1, 2);
    default_random_engine e(5);//e((int)time(0));

    int** mata, ** matb, ** matans, ** mattestans;
    mata = (int**)malloc(sizeof(int*) * arraySize);
    matb = (int**)malloc(sizeof(int*) * arraySize);
    matans = (int**)malloc(sizeof(int*) * arraySize);
    mattestans = (int**)malloc(sizeof(int*) * arraySize);
    for (int i = 0; i < arraySize; i++) {
        mata[i] = (int*)malloc(sizeof(int) * arraySize);
        matb[i] = (int*)malloc(sizeof(int) * arraySize);
        matans[i] = (int*)malloc(sizeof(int) * arraySize);
        mattestans[i] = (int*)malloc(sizeof(int) * arraySize);
    }
    for (int i = 0; i < arraySize; i++) {
        for (int j = 0; j < arraySize; j++) {
            mata[i][j] = u(e);
            matb[i][j] = u(e);
        }
    }

    // Fill matrices a, b, and c with values (omitted for brevity)

    double start_time, end_time;
    start_time = omp_get_wtime();
    int result_par = TestAns(mattestans, mata, matb, arraySize);
    end_time = omp_get_wtime();
    double time_par = end_time - start_time;
    printf("Parallel execution time: %f seconds\n", time_par);
    // Sequential
    start_time = omp_get_wtime();
    int result_seq = sequentialTestAns(matans, mata, matb, arraySize);
    end_time = omp_get_wtime();
    double time_seq = end_time - start_time;
    printf("Sequential execution time: %f seconds\n", time_seq);

    // Parallel

    // Calculate speedup
    double speedup = time_seq / time_par;
    printf("Speedup: %f\n", speedup);
    for (int i = 0; i < arraySize; i++)
        for (int j = 0; j < arraySize; j++)
            if (mattestans[i][j] != matans[i][j])
                printf("fuck You");

    return 0;
}