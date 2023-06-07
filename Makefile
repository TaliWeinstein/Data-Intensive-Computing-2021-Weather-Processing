# Compiler settings
CC = g++ -I/usr/local/Cellar/open-mpi/4.1.0/include -L/usr/local/opt/libevent/lib -L/usr/local/Cellar/open-mpi/4.1.0/lib -lmpi
CFLAGS = -fopenmp -lgomp -lpthread

# Compilation dependencies
ELEN4020_Project: main.o Array1D.o
	$(CC) $(CFLAGS) main.o Array1D.o -o ELEN4020_Project

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp

# Helper_Functions.o: Helper_Functions.cpp Helper_Functions.h
# 	$(CC) $(CFLAGS) -c Helper_Functions.cpp

# OMP_Functions.o: OMP_Functions.cpp OMP_Functions.h
# 	$(CC) $(CFLAGS) -c OMP_Functions.cpp

Array1D.o: Array1D.cpp Array1D.h
	$(CC) -c Array1D.cpp

# PThread_Functions.o: PThread_Functions.cpp PThread_Functions.h
# 	$(CC) $(CFLAGS) -c PThread_Functions.cpp

# Block.o: Block.cpp Block.h
# 	$(CC) $(CFLAGS) -c Block.cpp

# BlockArray.o: BlockArray.cpp BlockArray.h
# 	$(CC) $(CFLAGS) -c BlockArray.cpp

# Clean build
clean:
	rm -rf output
	rm *.o program