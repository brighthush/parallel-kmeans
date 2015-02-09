#include "io.h"

#include <stdio.h>
#include "comm.h"

void writeVisual(char *out_path, ProcData *proc)
{
    FILE *fout = fopen(out_path, "w");
    // num_clusters num_rows num_cols
    fprintf(fout, "\tnum_clusters num_rows num_cols\n");
    fprintf(fout, "%d %d %d\n", proc->num_clusters, proc->num_rows, proc->num_cols);
    int i, j;
    // centeroids of each cluster
    fprintf(fout, "\tcentroid of each cluster\n");
    for(i=0; i<proc->num_clusters; ++i) for(j=0; j<proc->num_cols; ++j)
        fprintf(fout, "%.08lf%c", proc->cens[i * proc->num_cols + j], j==(proc->num_cols-1)?'\n':' ');
    // points
    fprintf(fout, "\tposition of each point\n");
    for(i=0; i<proc->num_rows; ++i) 
    {
        fprintf(fout, "%d ", proc->clusters[i]);
        for(j=0; j<proc->num_cols; ++j)
            fprintf(fout, "%.08lf%c", proc->data[i * proc->num_cols + j], j==(proc->num_cols-1)?'\n':' ');
    }
    fclose(fout);
}

