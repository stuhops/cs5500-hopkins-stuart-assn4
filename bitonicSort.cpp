#include <iostream>
#include <mpi.h>
#include <unistd.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD

using namespace std;

void allPrint(int data) {
  int array[64];
  int rank, size;

  MPI_Comm_rank(MCW, &rank);
  MPI_Comm_size(MCW, &size);

  MPI_Gather(&data, 1, MPI_INT, array, 1, MPI_INT, 0, MCW);
  if(!rank){
    for(int i = 0; i < size; ++i) cout << i << " ";
    cout<<endl;
    for(int i = 0; i < size; ++i) cout << array[i] << " ";
    cout<<endl;
  }
  return;
}


bool increasing(int level){
  int rank, size;
  int mask = 2;  // Starts at 010 because first round already "done"
  mask <<= level;
  // cout << "\nMask " << mask << endl;

  MPI_Comm_rank(MCW, &rank);
  MPI_Comm_size(MCW, &size);

  // cout << "And " << (rank & mask) << endl;
  return !(rank & mask);
}


void cube(int f, bool inc, int *data){
  int rank, size;
  int dest;
  int mask=1;
  mask <<= f;

  MPI_Comm_rank(MCW, &rank);
  MPI_Comm_size(MCW, &size);

  dest = rank ^ mask;

  int myData = *data;

  // Should we send an array saying what processor it came from?
  MPI_Send(data, 1, MPI_INT, dest, 0, MCW);
  MPI_Recv(data, 1, MPI_INT, MPI_ANY_SOURCE, 0, MCW,MPI_STATUS_IGNORE);

  if(inc) {
    if(*data < myData && rank > dest)  // Change it back because it overrode it incorrectly
      *data = myData;
    else if(*data >= myData && rank < dest)  // Change it back because it overrode it incorrectly
      *data = myData;
  }
  else {
    if(*data > myData && rank > dest)  // Change it back because it overrode it incorrectly
      *data = myData;
    else if(*data <= myData && rank < dest)  // Change it back because it overrode it incorrectly
      *data = myData;
  }

  return;
}

int main(int argc, char **argv) {

  int rank, size;
  int data;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MCW, &rank);
  MPI_Comm_size(MCW, &size);

  data = (size - rank);

  allPrint(data);

  MPI_Barrier(MCW);
  if(!rank) cout << endl;
  for(int i = 0; i <=(size >> 2); i++) {  // x is equal to the number of bits plus 1 (aka 8 processors x = 4 and 16 processors x = 5)
    bool inc = increasing(i);

    for(int j = i; j >= 0; j--) {
      MPI_Barrier(MCW);
      cube(j, inc, &data);
    }
  }
  allPrint(data);

  MPI_Finalize();
  return 0;
}
