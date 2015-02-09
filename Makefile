./bin/main : ./src/kmeans.c ./src/comm.c
	mpicc -o ./bin/main ./src/kmeans.c ./src/comm.c -I./src

run : ./bin/main
	mpirun -np 3 ./bin/main

kmeans.c  : 
	vi ./src/kmeans.c


