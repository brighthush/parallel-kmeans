mpi-kmeans : mpi-kmeans.c
	mpicc -o mpi-kmeans mpi-kmeans.c

runmpi :
	mpirun -np 2 ./mpi-keans

run :
	./mpi-kmeans --num_clusters 3 --data_path './'
