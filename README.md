# parallel-kmeans
Using MPI to implement parallel kmeans algorithm. It means that this is a parallel implementation
that you can you can deal with much larger data set.  

## project introduction  
src : c source code is in this directory.  
visual : every iteration, the middle result is output in this directory, then use these output to do visiualization using
python matplotlib.  
python : python code to do visulization using matplotlib.  

## visualization of algorithm for random generated data  
In this experiment, I generated centroids for three clusters. Then randomly generated 500 points, each  
point randly belog to one cluster, then generate a distance to this centroid.  
Using the implemented mpi-kmeans to cluster these data, for every iteration, save the out of kmeans as 
a png file. The result is showd below.  

![iteration-1](python/iteration-1.png "iteration-1.png")  
![iteration-2](python/iteration-2.png "iteration-2.png")  
![iteration-3](python/iteration-3.png "iteration-3.png")  

