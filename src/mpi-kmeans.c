/*
 * @author : bright hush
 * @e-mail : brighthush@sina.com
 * @desc : The implementation of parallel k-means algorithm, using MPI technique.
 */

#include "mpi.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define STR_SIZE 100
#define MASTER 0

typedef long long int ll;
typedef double real;

typedef struct PROC_DATA
{
    int id, num_procs;
    ll num_rows, num_cols;
    real *data;
    char *rowname;

    int num_clusters, *clusters, *pre_clusters;
    real *cens;
} ProcData;

double randRange(double left, double right)
{
    double r = rand(); r = r / (double)RAND_MAX;
    r *= (right - left); r += left;
    return r;
}

void randData(int num_clusters, int num_rows, int num_cols, real *data)
{
    printf("begin to randData with:\n\t\
            num_clusters=%d, num_rows=%d, num_cols=%d.\n", \
            num_clusters, num_rows, num_cols);
    int c, i, j;
    for(i=0; i<num_rows; ++i) for(j=0; j<num_cols; ++j)
    {
        c = rand() % num_clusters;
        real center = c * 20.0;
        data[i * num_cols + j] = center + randRange(0, 5.0);
    }
    //for(i=0; i<num_rows; ++i) for(j=0; j<num_cols; ++j)
    //    printf("%lf%c", data[i * num_cols + j], j==num_cols-1?'\n':' ');
    printf("finished randData.\n");
}

void readHead(ll *num_rows, ll *num_cols, real **data, char **rowname)
{
    *num_rows = 10;
    *num_cols = 2;
    int rows = *num_rows, cols = *num_cols;
    *data = (real *)malloc(sizeof(real) * rows * cols);
    if(*data == NULL) { printf("malloc data failed.\n"); exit(-1); }
    *rowname = (char *)malloc((ll)sizeof(char) * (ll)rows * (ll)STR_SIZE);
    if(*rowname == NULL) { printf("malloc rowname failed.\n"); exit(-1); }
}

int argPos(char *arg, int argc, char *argv[])
{
    int pos = -1;
    int i = 0;
    for(; i<argc; ++i)
    {
        if(strcmp(arg, argv[i]) == 0) { pos = i + 1; break; }
    }
    if(pos < argc) return pos;
    else return -1;
}

void initMaster(ProcData *proc)
{
    printf("initMaster %d %d ...\n", proc->id, proc->num_procs);
    readHead(&(proc->num_rows), &(proc->num_cols), &(proc->data), &(proc->rowname));
    randData(proc->num_clusters, proc->num_rows, proc->num_cols, proc->data);
    proc->cens = (real *)malloc((ll)sizeof(real) * proc->num_clusters * proc->num_cols);
    if(proc->cens==NULL) { printf("malloc cens faild in MASTER.\n"); exit(-1); }
    proc->clusters = (int *)malloc((ll)sizeof(int) * proc->num_rows);
    if(proc->clusters == NULL) { printf("malloc clusters faild in MASTER.\n"); exit(-1); }
    proc->pre_clusters = (int *)malloc((ll)sizeof(int) * proc->num_rows);
    if(proc->pre_clusters == NULL) { printf("malloc pre_clusters faild in MASTER.\n"); exit(-1); }
    int num_slaves = proc->num_procs - 1;
    int size = proc->num_rows / num_slaves;
    int i, send_tag;
    for(i=1; i<=num_slaves; ++i)
    {
        send_tag = 0;
        int left = (i-1) * size;
        int right = i * size;
        if(i == num_slaves) right = proc->num_rows;
        MPI_Send(&(proc->num_clusters), 1, MPI_INT, i, send_tag++, MPI_COMM_WORLD);           // send num_clusters
        int size = right - left;
        MPI_Send(&size, 1, MPI_LONG_LONG_INT, i, send_tag++, MPI_COMM_WORLD);                 // send num_rows
        MPI_Send(&(proc->num_cols), 1, MPI_LONG_LONG_INT, i, send_tag++, MPI_COMM_WORLD);     // send num_cols
        printf("send data [%d, %d) in initMaster.\n", left, right);
        MPI_Send((void *)(proc->data + left * proc->num_cols), size * (proc->num_cols), \
                MPI_DOUBLE, i, send_tag++, MPI_COMM_WORLD);                                   // send row data
    }
    printf("finished initMaster.\n");
}

void initSlave(ProcData *proc)
{
    printf("initSlave %d/%d slave ...\n", proc->id, proc->num_procs);
    MPI_Status status;
    int recv_tag = 0;
    MPI_Recv(&(proc->num_clusters), 1, MPI_INT, MASTER, recv_tag++, MPI_COMM_WORLD, &status); // recv num_clusters
    MPI_Recv(&(proc->num_rows), 1, MPI_LONG_LONG_INT, MASTER, recv_tag++, MPI_COMM_WORLD, &status);     // recv num_rows
    MPI_Recv(&(proc->num_cols), 1, MPI_LONG_LONG_INT, MASTER, recv_tag++, MPI_COMM_WORLD, &status);     // recv num_cols
    printf("porcess %d, get num_rows=%lld, num_cols=%lld\n", proc->id, proc->num_rows, proc->num_cols);
    proc->data = (real *)malloc((ll)sizeof(real) * (proc->num_rows) * (proc->num_cols));
    if(proc->data == NULL) { printf("malloc data failed in slave %d\n", proc->id); exit(-1); }
    MPI_Recv((void*)proc->data, proc->num_rows * proc->num_cols, MPI_DOUBLE, MASTER, \
            recv_tag++, MPI_COMM_WORLD, &status);                                             // recv data
    proc->cens = (real *)malloc((ll)sizeof(real) * proc->num_clusters * proc->num_cols);
    if(proc->cens == NULL) { printf("malloc cens faild in slave %d.\n", proc->id); exit(-1); }
    proc->clusters = (int *)malloc((ll)sizeof(int) * proc->num_rows);
    if(proc->clusters == NULL) { printf("malloc clusters faild in slave %d.\n", proc->id); exit(-1); }
    printf("finished initSlave %d.\n", proc->id);
}

int main(int argc, char* argv[])
{
    srand(time(NULL));
    ProcData proc; 
    //readHead(&num_rows, &num_cols, &data, &rowname);
    //randData(num_clusters, num_rows, num_cols, data);
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &(proc.id));
    MPI_Comm_size(MPI_COMM_WORLD, &(proc.num_procs));
    if(proc.id == 0)
    {
        proc.num_clusters = 3;
        initMaster(&proc);
    }
    else
    {
        initSlave(&proc);
    }
    MPI_Finalize();
    return 0;
}

