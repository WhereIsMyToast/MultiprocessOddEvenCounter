#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

int* randomNumberGenerator(int n);
int* oddEvenCounter(int numbers[], int start, int end);

int main(int argc, char const *argv[]) {
    // Check if necessary arguments exist
    if(argc < 3) {
        printf("Invalid arguments\nUsage: %s <integer, numbers to generate> <integer, processes to use>\n", argv[0]);
        return EXIT_FAILURE;
    }
    // Check if values are greater than 1
    int numbersToGenerate = atoi(argv[1]);  
    int childProcesses = atoi(argv[2]);
    if(numbersToGenerate < 1 || childProcesses < 1) {
        printf("Enter valid values\nInvalid arguments\nUsage: %s <integer, numbers to generate> <integer, processes to use>\n", argv[0]);
        return EXIT_FAILURE;
    }
    // Generate numeric values by calling local function
    int *numbers = randomNumberGenerator(numbersToGenerate);
    // Create pipes for each process
    int fd[2 * childProcesses];
    // Initialize pipes and check successful creation
    for(int i = 0; i < childProcesses; i++) {
        if(pipe(&fd[2 * i]) != 0) {
            printf("Error creating pipe\n");
            return EXIT_FAILURE;
        }
    }
    // Main loop, iterates childProcesses times
    for(int i = 0; i < childProcesses; i++) {
        // Create child process using fork method and store id
        pid_t pid = fork();
        if(pid == 0) {
            // Enter child process sequence
            // Close input pipe
            close(fd[2 * i]);
            // Calculate start and end for each process
            int start = i * (numbersToGenerate / childProcesses);  
            int end = (i + 1) * (numbersToGenerate / childProcesses) - 1;
            // Handle end of last process
            if(i == childProcesses - 1) {
                end = numbersToGenerate - 1;
            }
            // Call method to perform the count
            int* result = oddEvenCounter(numbers, start, end);
            // Write result into the dedicated pipe for the process
            write(fd[2 * i + 1], result, 2 * sizeof(int));
            // Free memory of results
            free(result);
            printf("Process with id %d has finished\n", getpid());
            return 0;
        }
    }
    // Loop to wait for each child process
    for(int i = 0; i < childProcesses; i++) {
        wait(NULL);
    }
    // Loop for total sum
    int numEven = 0, numOdd = 0;    
    for(int i = 0; i < childProcesses; i++) {
        // Close write pipe
        close(fd[2 * i + 1]);
        int temp[2];
        // Read values from the pipe and store them
        read(fd[2 * i], temp, sizeof(int) * 2);
        numEven += temp[0];
        numOdd += temp[1];
    }
    // Free memory of numbers
    free(numbers);
    printf("The number of generated numbers is %d, the total of even numbers is %d, and odd numbers is %d, the sum of these is %d\n", numbersToGenerate, numEven, numOdd, numEven + numOdd);
    return 0;
}

int* randomNumberGenerator(int n) {
    // Allocate memory for the numbers
    int* nums = malloc(n * sizeof(int));
    // Seed srand
    srand((unsigned)time(NULL));
    // Loop to generate all numbers
    for(int i = 0; i < n; i++) {
        nums[i] = rand() % 100; 
    }
    return nums; 
}

int* oddEvenCounter(int numbers[], int start, int end) {
    // Allocate memory for the result
    int* result = malloc(2 * sizeof(int));
    // Create counters
    int counterEven = 0, counterOdd = 0;
    // Count even and odd numbers
    for(int i = start; i <= end; i++) {
        if(numbers[i] % 2 == 0) {
            counterEven++;
        } else {
            counterOdd++;
        }
    }
    // Assign counters to result
    result[0] = counterEven; 
    result[1] = counterOdd; 
    printf("The process with pid %d has finished with a result of %d even numbers and %d odd numbers\n", getpid(), counterEven, counterOdd);
    return result;
}
