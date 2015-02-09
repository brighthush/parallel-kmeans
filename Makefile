./bin/main : ./src/kmeans.c ./src/comm.c ./src/io.c
	mpicc -o ./bin/main ./src/kmeans.c ./src/comm.c ./src/io.c -I./src

run : ./bin/main
	mpirun -np 3 ./bin/main

kmeans.c  : 
	vi ./src/kmeans.c


