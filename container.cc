#include <iostream>
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

/**
 * @brief called from the jail method, spawns a shell.
 * called by: run("/bin/sh");
 *
 * @param name the command to exec
 */
int run(const char *name) {
    char *_args[] = {(char *)name, (char *)0 };
    return execvp(name, _args);
}

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

/**
 * @brief handles the env variables of the new container
 *
 */
void setup_variables() {
    clearenv();
    setenv("TERM", "xterm-256color", 0);
    setenv("PATH", "/bin/:/sbin/:usr/bin:/usr/sbin", 0);
}

/**
 * @brief Set the up root object and performs the chroot for filesystem isolation
 *
 * @param folder the path of the folder to be root
 */
void setup_root(const char* folder){
    chroot(folder);
    chdir("/");
}

/**
 * @brief the child process that is called from clone
 *
 * @param args any optional args sent from clone
 * @return int EXIT_STATUS
 */
int jail(void *args) {
    printf("child process: %d", getpid());

    setup_variables();
    setup_root("./root");

    run("/bin/sh");
    return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
    printf("parent process: %d\n", getpid());

    // creates the child process
    // NOTE: sementics of clone differ from fork() in that the child process starts
    // by calling the function defined as the first parameter
    // CLONE_NEWPID -
    // CLONE_NEWUTS -
    // SIGCHLD - flag that tells the process to emit a signal when finished
    // 0 - parameter we send to jail()

    clone(jail, stack_memory(), CLONE_NEWPID | CLONE_NEWUTS | SIGCHLD, 0);
    wait(nullptr);

    return EXIT_SUCCESS;
}