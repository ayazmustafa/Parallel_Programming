#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define COMM MPI_COMM_WORLD
#define TAG 0

static int bit(uint32_t v);

int myReduce(int value) {
 
    int recvbuffer;
    int rank, size;
    MPI_Comm_rank(COMM, &rank);
    MPI_Comm_size(COMM, &size);
    //int a = bit(size);
    int last = 1 << bit(size);
    //printf("%d", a);
    //printf("%d", last);
    MPI_Status* stat = (MPI_Status*)malloc(sizeof(MPI_Status));

    // eger rank en sondaki 2^n den b�y�kse ve size'dan kucukse, bi anda downshift ederiz o datay� ayn� tree yap�s�nda oldugu gibi
    // ki sadece 2^n'de �al��s�n.

    for (int i = last; i < size; i++)
        if (rank == i) {
            MPI_Send(&value, 1, MPI_INT, i - last, TAG, COMM);
        }
    for (int i = 0; i < size - last; i++)
        if (rank == i) {
            MPI_Recv(&recvbuffer, 1, MPI_INT, i + last, TAG, COMM, stat);
            value += recvbuffer; 
        }

    for (int d = 0; d < bit(last); d++)
        for (int k = 0; k < last; k += 1 << (d + 1)) {
            int receiver = k;
            int sender = k + (1 << d);
            if (rank == receiver) {
                MPI_Recv(&recvbuffer, 1, MPI_INT, sender, TAG, COMM, stat);
                value += recvbuffer;
            }
            else if (rank == sender)
                MPI_Send(&value, 1, MPI_INT, receiver, TAG, COMM);
        }

    return value;
}


// https://stackoverflow.com/questions/757059/position-of-least-significant-bit-that-is-set 
static int bit(uint32_t v) {
    int r;
    static const int MultiplyDeBruijnBitPosition[32] =
    {
      0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30,
      8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31
    };

    v |= v >> 1; 
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;

    r = MultiplyDeBruijnBitPosition[(uint32_t)(v * 0x07C4ACDDU) >> 27];
    return r;
}



int main(void) {
    MPI_Init(NULL, NULL);
    int rank, size;
    MPI_Comm_rank(COMM, &rank);
    MPI_Comm_size(COMM, &size);
    double startTime = MPI_Wtime();

    // 1'den size'a kadar atanan do�al say�lar.
    int* data = (int*)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++)  
        data[i] = i;
    int result = myReduce(data[rank]);
    



    // Dogrulugunu saglamak icin kendim basit bir dongu yapt�m, fonksiyonumdaki sonucla eslesiyor mu diye.
    int sum = 0;
    for (int i = 0; i < size; i++)
        sum += data[i];
    

    // bu max s�reyi almam� sagliyor. cunku butun process'ler beklemek zorunda.
    MPI_Barrier(COMM);

    
    double finishTime = MPI_Wtime();
    double elapsedTime = finishTime - startTime;

    if (rank == 0) {
        printf("MPI Result     = %d\n",             result);
        printf("Correct Result = %d\n",             sum);
        printf("Elapsed time   = %f sec./n",        elapsedTime);
    }

    MPI_Finalize();


}

