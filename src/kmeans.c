/*
 * @author : BrightHush
 * @e-mail : brighthush@sina.com
 * @desc : The implementation of parallel k-means algorithm, using MPI technique.
 */

#include "mpi.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "comm.h"
#include "io.h"

void randData(int num_clusters, int num_rows, int num_cols, real *data)
{
    printf("begin to randData with:\n\t\
            num_clusters=%d, num_rows=%d, num_cols=%d.\n", \
            num_clusters, num_rows, num_cols);
    int c, i, j;
    for(i=0; i<num_rows; ++i)
    {
        c = rand() % num_clusters;
        real center = c * 20.0;
        for(j=0; j<num_cols; ++j)
        {
            data[i * num_cols + j] = center + randRange(0, 20.0);
        }
    }
    //for(i=0; i<num_rows; ++i) for(j=0; j<num_cols; ++j)
    //    printf("%lf%c", data[i * num_cols + j], j==num_cols-1?'\n':' ');
    printf("finished randData.\n");
}

void readHead(int *num_rows, int *num_cols, real **data, char **rowname)
{
    *num_rows = 500;
    *num_cols = 2;
    int rows = *num_rows, cols = *num_cols;
    *data = (real *)malloc(sizeof(real) * rows * cols);
    if(*data == NULL) { printf("malloc data failed.\n"); exit(-1); }
    *rowname = (char *)malloc((ll)sizeof(char) * (ll)rows * (ll)STR_SIZE);
    if(*rowname == NULL) { printf("malloc rowname failed.\n"); exit(-1); }
}

void processCens(ProcData *proc)
{
    printf("begin processCens in %d.\n", proc->id);
    if(proc->id == MASTER) // MASTER send centers' infomation to slaves
    {
        int i;
        for(i=1; i<proc->num_procs; ++i)
        {
            MPI_Send((void *)proc->cens, proc->num_clusters * proc->num_cols, MPI_DOUBLE, \
                    i, 0, MPI_COMM_WORLD);
        }
    }
    else                   // slaves recv centers' information from MASTER
    {
        MPI_Status status;
        MPI_Recv((void *)proc->cens, proc->num_clusters * proc->num_cols, MPI_DOUBLE, \
                MASTER, 0, MPI_COMM_WORLD, &status);
    }
    printf("finished processCens in %d.\n", proc->id);
}

void processClusters(ProcData *proc)
{
    printf("begin processClusters in %d.\n", proc->id);
    if(proc->id == MASTER) // MASTER recv the clusters for every point
    {
        int i, num_slaves = proc->num_procs - 1;
        int size = proc->num_rows / num_slaves;
        MPI_Status status;
        for(i=1; i<=num_slaves; ++i)
        {
            int left = (i-1) * size;
            int right = i * size;
            if(i == num_slaves) right = proc->num_rows;
            MPI_Recv((void *)(proc->clusters + left), right - left, MPI_INT, \
                    i, 0, MPI_COMM_WORLD, &status);
        }
    }
    else // slaves send clusters for every data point
    {
        MPI_Send((void *)(proc->clusters), proc->num_rows, MPI_INT, MASTER, 0, MPI_COMM_WORLD);
    }
    printf("finished processClusters %d.\n", proc->id);
}

void processDone(ProcData *proc)
{
    printf("begin processDone in %d.\n", proc->id);
    if(proc->id == MASTER) // send done flag
    {
        int i;
        for(i=1; i<proc->num_procs; ++i)
        {
            MPI_Send(&(proc->done), 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    }
    else                   // slaves recv done flag
    {
        MPI_Status status;
        MPI_Recv(&(proc->done), 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD, &status);
    }
    printf("finished processDone in %d.\n", proc->id);
}

void initMaster(ProcData *proc)
{
    printf("initMaster %d %d ...\n", proc->id, proc->num_procs);
    proc->done = 0;
    readHead(&(proc->num_rows), &(proc->num_cols), &(proc->data), &(proc->rowname));
    randData(proc->num_clusters, proc->num_rows, proc->num_cols, proc->data);
    proc->cens = (real *)malloc((ll)sizeof(real) * proc->num_clusters * proc->num_cols);
    if(proc->cens==NULL) { printf("malloc cens faild in MASTER.\n"); exit(-1); }
    // randomly init center for each cluster
    int i, j;
    for(i=0; i<proc->num_clusters; ++i) for(j=0; j<proc->num_cols; ++j)
        proc->cens[i * proc->num_cols + j] = proc->data[i * proc->num_cols + j];
    printf("\t finished randon proc->cens.\n");
    // init clusters and pre_clusters
    proc->clusters = (int *)malloc((ll)sizeof(int) * proc->num_rows);
    if(proc->clusters == NULL) { printf("malloc clusters faild in MASTER.\n"); exit(-1); }
    proc->pre_clusters = (int *)malloc((ll)sizeof(int) * proc->num_rows);
    if(proc->pre_clusters == NULL) { printf("malloc pre_clusters faild in MASTER.\n"); exit(-1); }
    memset(proc->clusters, -1, proc->num_clusters);
    memset(proc->pre_clusters, -1, proc->num_clusters);
    printf("\t finished randon proc->clusters.\n");
    // send data to slaves
    int num_slaves = proc->num_procs - 1;
    int size = proc->num_rows / num_slaves;
    int send_tag;
    for(i=1; i<=num_slaves; ++i)
    {
        send_tag = 0;
        int left = (i-1) * size;
        int right = i * size;
        if(i == num_slaves) right = proc->num_rows;
        MPI_Send(&(proc->num_clusters), 1, MPI_INT, i, send_tag++, MPI_COMM_WORLD);           // send num_clusters
        int size = right - left;
        MPI_Send(&size, 1, MPI_INT, i, send_tag++, MPI_COMM_WORLD);                 // send num_rows
        MPI_Send(&(proc->num_cols), 1, MPI_INT, i, send_tag++, MPI_COMM_WORLD);     // send num_cols
        printf("send data [%d, %d) in initMaster.\n", left, right);
        MPI_Send((void *)(proc->data + left * proc->num_cols), size * (proc->num_cols), \
                MPI_DOUBLE, i, send_tag++, MPI_COMM_WORLD);                                   // send row data
    }
    printf("finished initMaster.\n");
}

void initSlave(ProcData *proc)
{
    printf("initSlave %d/%d slave ...\n", proc->id, proc->num_procs-1);
    MPI_Status status;
    int recv_tag = 0;
    MPI_Recv(&(proc->num_clusters), 1, MPI_INT, MASTER, recv_tag++, MPI_COMM_WORLD, &status); // recv num_clusters
    MPI_Recv(&(proc->num_rows), 1, MPI_INT, MASTER, recv_tag++, MPI_COMM_WORLD, &status);     // recv num_rows
    MPI_Recv(&(proc->num_cols), 1, MPI_INT, MASTER, recv_tag++, MPI_COMM_WORLD, &status);     // recv num_cols
    printf("porcess %d, get num_rows=%d, num_cols=%d\n", proc->id, proc->num_rows, proc->num_cols);
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

int checkDone(ProcData *proc)
{
    printf("checkDone in %d.\n", proc->id);
    int changed = 0, i;
    for(i=0; i<proc->num_rows; ++i)
        if(proc->clusters[i] != proc->pre_clusters[i]) ++changed;
    real ratio = (real)changed / (real)proc->num_rows;
    if(ratio < CONVERGE) return 1;
    else
    {
        for(i=0; i<proc->num_rows; ++i) proc->pre_clusters[i] = proc->clusters[i];
        return 0;
    }
    printf("finished checkDone in %d.\n", proc->id);
}

void display(ProcData *proc)
{
    int i, j;
    printf("=====================cluster info===========================\n");
    for(i=0; i<proc->num_rows; ++i)
    {
        for(j=0; j<proc->num_cols; ++j) printf("%.05lf ", proc->data[i * proc->num_cols + j]);
        printf("cluster : %d[", proc->clusters[i]);
        for(j=0; j<proc->num_cols; ++j) printf("%.05lf ", proc->cens[(proc->clusters[i]) * proc->num_cols + j]);
        printf("]\n");
    }
    printf("==========================end===============================\n");
}

int iteration = 0;
void kmeans(ProcData *proc)
{
    if(proc->id == MASTER)
    {
        int *cluster_cnt = (int *)malloc(sizeof(int) * proc->num_clusters);
        while(1)
        {
            processDone(proc);
            if(proc->done) break;
            processCens(proc);
            processClusters(proc);
            display(proc);
            int i, j, k;
            for(i=0; i<proc->num_clusters; ++i) cluster_cnt[i] = 0;
            for(i=0; i<proc->num_clusters; ++i) for(j=0; j<proc->num_cols; ++j)
                proc->cens[i * proc->num_cols + j] = 0;
            for(k=0; k<proc->num_rows; ++k)
            {
                int clu = proc->clusters[k];
                ++cluster_cnt[clu];
                for(i=0; i<proc->num_cols; ++i)
                    proc->cens[clu * proc->num_cols + i] += proc->data[k * proc->num_cols + i];
            }
            for(i=0; i<proc->num_clusters; ++i) for(j=0; j<proc->num_cols; ++j)
            {
                if(cluster_cnt[i] == 0)
                {
                    int r = randInt(0, proc->num_rows - 1);
                    for(j=0; j<proc->num_cols; ++j)
                        proc->cens[i * proc->num_cols + j] = proc->data[r * proc->num_cols + j];
                }
                else
                {
                    for(j=0; j<proc->num_cols; ++j)
                        proc->cens[i * proc->num_cols + j] /= (real)cluster_cnt[i];
                }
            }
            proc->done = checkDone(proc);
            ++iteration;
            char out_path[100];
            sprintf(out_path, "./visual/iteration-%d", iteration);
            printf("out_path %s\n", out_path);
            writeVisual(out_path, proc);
        }
    }
    else
    {
        while(1)
        {
            processDone(proc);
            if(proc->done) break;
            processCens(proc);
            int i, j, k;
            for(i=0; i<proc->num_rows; ++i)
            {
                int clu = -1;
                real dis;
                for(j=0; j<proc->num_clusters; ++j)
                {
                    real temp = dist(proc->data + i * proc->num_cols, proc->cens + j * proc->num_cols, proc->num_cols);
                    if(clu == -1) { clu = j; dis = temp; }
                    else if(dis > temp) { clu = j; dis = temp; }
                }
                proc->clusters[i] = clu;
            }
            processClusters(proc);
        }
    }
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
    kmeans(&proc);
    MPI_Finalize();
    return 0;
}

