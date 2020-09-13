#include <iostream>
#include <sched.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <fstream>

#define CGROUP_FOLDER "/sys/fs/cgroup/pids/container/"
#define concat(a,b) (a"" b)
#define NUM_PROCESSES_ALLOWED "5"

/**
 * @brief custom try block
 *
 * @param status
 * @param msg
 * @return int
 */
int TRY(int status, const char *msg) {
    if(status == -1) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
    return status;
}

/**
 * @brief writes the process id to cgroup.procs
 *
 * @param path
 * @param value
 */
void write_rule(const char* path, const char* value) {
    int fp = open(path, O_WRONLY | O_APPEND );
    write(fp, value, strlen(value));
    close(fp);
}

/**
 * @brief use cgroups to limit the amount of processes we can create inside our container
 * the control group called pids controller can be used to limit the amount of
 * times a process can replicate itself, for example using fork or clone.
 *
 */
void limitProcessCreation() {
    mkdir( CGROUP_FOLDER, S_IRUSR | S_IWUSR);  // Read & Write
    const char* pid  = std::to_string(getpid()).c_str();

    write_rule(concat(CGROUP_FOLDER, "pids.max"), NUM_PROCESSES_ALLOWED);
    write_rule(concat(CGROUP_FOLDER, "notify_on_release"), "1");
    write_rule(concat(CGROUP_FOLDER, "cgroup.procs"), pid);
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
 * @brief Set the Host Name of the container
 *
 * @param hostname
 */
void setHostName(std::string hostname) {
    sethostname(hostname.c_str(), hostname.size());
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
 * @brief Set the up root object and performs the chroot for filesystem isolation
 *
 * @param folder the path of the folder to be root
 */
void setup_root(const char* folder){
    chroot(folder);
    chdir("/");
}

/**
 * @brief creates the child process
 * creates a new “generic type” called Function which will morph into a C function,
 * then we pass the function to clone, also we pass the flags as an integer.
 * @tparam Function
 * @param function - in this case, it is used to call jail
 * @param flags - the falgs to send clone
 */
template <typename Function>
void clone_process(Function&& function, int flags){
    auto pid = clone(function, stack_memory(), flags, 0);

    wait(nullptr);
}

/**
 * @brief the child process that is called from clone
 *
 * @param args any optional args sent from clone
 * @return int EXIT_STATUS
 */
#define lambda(fn_body) [](void *args) ->int { fn_body; };
int jail(void *args) {

    // Limits the number of process that can be spawned in the continer to 5
    // but its really more like 3, because container and sh take up 2 of them
    limitProcessCreation();
    printf("child pid: %d\n", getpid());
    setHostName("my-container");
    setup_variables();
    setup_root("./root");
    mount("proc", "/proc", "proc", 0, 0);

    // lambda "runnable" that is passed into the clone_process function
    auto runnable = lambda(run("/bin/sh"))
    clone_process(runnable, SIGCHLD);

    umount("/proc");
    return EXIT_SUCCESS;
}

int main(int argc, char** argv) {

    printf("parent pid: %d\n", getpid());
    clone_process(jail, CLONE_NEWPID | CLONE_NEWUTS | SIGCHLD );

    return EXIT_SUCCESS;
}