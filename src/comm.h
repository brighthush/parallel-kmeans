#ifndef __COMM__H__
#define __COMM__H__

#define STR_SIZE 100
#define MASTER 0
#define CONVERGE (0.1)

typedef long long int ll;
typedef double real;

typedef struct PROC_DATA
{
    int id, num_procs;
    int num_rows, num_cols;
    real *data;
    char *rowname;
    // the number of clusters, each point belongs to which cluster, 
    // the previous cluster each point belongs to
    int num_clusters, *clusters, *pre_clusters;
    real *cens;
    // algorithm finished
    int done;
} ProcData;

int argPos(char *arg, int argc, char *argv[]);
double randRange(double left, double right);  // rand a double in range [left, right]
int randInt(int left, int right); // rand a int in range [left, right]
real dist(real *veca, real *vecb, int size);

#endif
