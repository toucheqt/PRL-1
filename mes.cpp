/**
 * Project: Minimum extraction sort using MPI
 * Author: Ondøej Krpec, xkrpec01@stud.fit.vutbr.cz
 * Date: 29.3.2016
 * Description: Implementation of minimum extraction sort as second project for subject PRL lectured at FIT VUT in Brno.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string>     
#include <mpi.h>         

// define custom Not a Number constant, standard does not work
#define NaN -1 

// define breakpoint for sorting algorithm 
#define StopSign -2

using namespace std;

/**
 * Enumaration type for error codes in application.
 */
enum tErrCodes {
    EOK = 0,
    EFILE
};

/**
 * Struct for representing tree structure with addition of stopping (child) processes
 */
struct tree {
    int number;
    int parentNumber;
    int leftChild, rightChild;    
    bool stop, stopAll;
    
    tree() : number(NaN), parentNumber(0), leftChild(0), rightChild(0), stop(false), stopAll(false) {};
};

/**
 * Reads sequence of numbers from binary file. All numbers are printed to stdout and send to lists of tree.
 * @param int Id of current cpu (process).
 * @param int Number of cpus given by input parameter.
 * @param char* Filename of binary file that should contain sequence of char numbers.
 * @return int Returns status code of function. If OK, returns 0.
 */
int prepareNumSequence(int processCounter, int cpuCount, char* filename);

/**
 * Implementation of sorting algorithm minimum extraction sort using MPI library. Function sorts input number sequence and prints each number on separate line to stdout.
 * @param counter Counter of cpus.
 * @param mpiRank Rank of main process.
 * @param sortTreee Struct containing informations about input tree.
 */
void minimumExtractionSort(int counter, int mpiRank, tree sortTree);

/**
 * Based on process rank computes the rank of parent nod.
 * @param rank Process rank
 * @return int Rank of the parent nod.
 */
int getParentRank(int rank);


int main(int argc, char* argv[]) {

    int mpiRank;
    int mpiSize;
    tree sortTree;
    
    int cpuCount = atoi(argv[2]);
    int processCounter = (((atoi(argv[2])) + 1) / 2) - 1;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
    
    if (mpiRank == 0) {
        if ((prepareNumSequence(processCounter, cpuCount, argv[1])) != 0)  {
          return EXIT_FAILURE; 
        }
    }
    
    minimumExtractionSort(processCounter, mpiRank, sortTree);
    MPI_Finalize();
    
    return EXIT_SUCCESS;    
}



int prepareNumSequence(int processCounter, int cpuCount, char* filename) {
    FILE* fHandle = NULL;
    if ((fHandle = fopen(filename, "r")) == NULL) {
        return EFILE;
    }
        
    int charAsInt;
    while ((charAsInt = getc(fHandle)) != EOF) {
        cout << charAsInt << " ";
        MPI_Send(&charAsInt, 1, MPI_INT, processCounter, 0, MPI_COMM_WORLD);
        processCounter++;
    }
       
    fclose(fHandle);   
    
    // fill in blank lists with NaN
    while (processCounter < cpuCount) {
        int tmp = NaN;
        MPI_Send(&tmp, 1, MPI_INT, processCounter, 0, MPI_COMM_WORLD);
        processCounter++;
    }   
    
    cout << "\n";  
    
    return EOK;
}



void minimumExtractionSort(int processCounter, int mpiRank, tree sortTree) {
    MPI_Status stat;               
                   
    if (mpiRank >= processCounter) { // recv is blocking operation, processes should wait for loading all numbers from input file
        MPI_Recv(&(sortTree.number), 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &stat);
    }
            
    while (!sortTree.stopAll) {
        if (mpiRank < processCounter) {
            if (mpiRank == 0 && sortTree.number != NaN) {
                cout << sortTree.number << "\n";
                sortTree.number = NaN;
            }
            
            if (sortTree.number == NaN && !sortTree.stop) {                
                MPI_Recv(&(sortTree.leftChild), 1, MPI_INT, mpiRank * 2 + 1, 0, MPI_COMM_WORLD, &stat);
                MPI_Recv(&(sortTree.rightChild), 1, MPI_INT, mpiRank * 2 + 2, 0, MPI_COMM_WORLD, &stat);  
                  
                if (sortTree.leftChild == NaN && sortTree.rightChild == NaN) {
                    sortTree.stop = true;
                    sortTree.leftChild = StopSign;
                    sortTree.rightChild = StopSign;
                    if (mpiRank == 0) {
                        sortTree.stopAll = true;
                    }     
                } else if ((sortTree.leftChild == NaN && sortTree.rightChild != NaN) || ((sortTree.leftChild > sortTree.rightChild) && sortTree.rightChild != NaN)) {
                    sortTree.number = sortTree.rightChild;
                    sortTree.rightChild = NaN;
                } else if ((sortTree.leftChild != NaN && sortTree.rightChild == NaN) || ((sortTree.leftChild <= sortTree.rightChild) && sortTree.leftChild != NaN)) {
                    sortTree.number = sortTree.leftChild;
                    sortTree.leftChild = NaN;
                }
                
                MPI_Send(&(sortTree.leftChild), 1, MPI_INT, mpiRank * 2 + 1, 0, MPI_COMM_WORLD);
                MPI_Send(&(sortTree.rightChild), 1, MPI_INT, mpiRank * 2 + 2, 0, MPI_COMM_WORLD);
            }
        }
        
        if (mpiRank != 0) {
            MPI_Send(&(sortTree.number), 1, MPI_INT, getParentRank(mpiRank), 0, MPI_COMM_WORLD);
            MPI_Recv(&(sortTree.number), 1, MPI_INT, getParentRank(mpiRank), 0, MPI_COMM_WORLD, &stat);  
            if (sortTree.number == StopSign) {
                break;
            }
        }     
    }
          
}



int getParentRank(int rank) {
    return (rank % 2) == 0 ? (rank - 2) / 2 : (rank - 1) / 2; 
}
