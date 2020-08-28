#include <iostream>
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>


/**
 * @brief allocates 65k bytes of memory
 *
 * @return char* the pointer to the end of the array
 */
char* stack_memory() {
    const int stackSize = 65536;
    auto *stack = new (std::nothrow) char[stackSize];

    if (stack == nullptr) {
        printf("Cannot allocate memory \n");
        exit(EXIT_FAILURE);
    }

    return stack+stackSize;  //move the pointer to the end of the array because the stack grows backward.
}

int jail(void *args) {
    printf("Hello !! ( child ) \n");
    return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
    printf("Hello, World! ( parent ) \n");

    // creates the child process
    // NOTE: sementics of clone differ from fork() in that the child process starts
    // by calling the function defined as the first parameter
    // SIGCHLD - flag that tells the process to emit a signal when finished
    // 0 - parameter we send to jail()
    clone(jail, stack_memory(), SIGCHLD, 0);
    wait(nullptr);

    return EXIT_SUCCESS;
}