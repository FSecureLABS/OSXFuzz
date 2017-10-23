//
//  main.c
//  OSXFuzz
// Usage ./OSXFuzz <seed> <logger dir> <syscall blacklist>
//
#define _XOPEN_SOURCE 600 //required for ucontext.h, which is deperectated but required for some syscall fuzzing.
#include <stdio.h>
#include <limits.h>
#include <getopt.h>

#include "object_database.h"
#include "bughunt_thread.h"

// for some reason we need to be 32bit for the mach stuff.

void start_fuzzing()
{
    logger("//Starting fuzzing!");
    bughunt_thread(350000);
}

void print_usage()
{
    printf("Usage: OSXFuzz -s <seed> -l <ip addr> -p <port> -b <blacklist> \n");
    printf("Usage: OSXFuzz -s <seed> -d <log dir> -b <blacklist> \n");
}

int main(int argc, char * argv[]) {
    
    int option = 0;
    unsigned int seed = 1;
    char *dir;
    char *blacklist;
    char cwd[PATH_MAX];
    
    // Get the current working directory.
    getcwd(cwd, sizeof(cwd));
    dir = cwd;
    
    // Initialize the logger to localhost by default
    logging_host = "127.0.0.1";
    logging_port = 1337;
    
    // Initialize the blacklist to fork and exit by default.
    //blacklist = "1,2";

    // Get the command line arguments
    while ((option = getopt(argc, argv,"s:d:l:b:p:D")) != -1)
    {
        switch (option)
        {
            case 's' : seed = atoi(optarg); break;
            case 'd' : dir = optarg; disk_logging = 1; break;
            case 'l' : logging_host = optarg; network_logging = 1; break;
            case 'p' : logging_port = atoi(optarg); break;
            case 'b' : blacklist = optarg; break;
            case 'D' : debug = 1; break; 
            default: print_usage(); break;
        }
    }
    
    if (!disk_logging && !network_logging)
    {
        printf("++ Disk logging or network logging must be specified ++\n");
        exit(EXIT_FAILURE);
    }
    
    // Currently we disallow both disk and networking logging.
    if (disk_logging && network_logging)
    {
        printf("++ Cannot set both logger dir and network logger ++\n");
        exit(EXIT_FAILURE);
    }
    
    // Initialize the logger to either a disk logger or network logger.
    if (disk_logging)
    {
        printf("++ Using disk logging mode ++\n");
        if (strcmp(dir,"-") == 0)
            dir = cwd;
        printf("Logger path: %s\n", dir);
        strncpy(logger_path,dir,sizeof(logger_path));
    }
    
    if (network_logging)
    {
        printf("++ Using networking logging mode ++\n");
        printf("Networking logging host is: %s",logging_host);
    }

    // Seed the PRNG
    if (seed == 1)
    {
        seed = (unsigned int)time(NULL) + getpid();
        //logger("//[PRNG Seed] (0x%08X, %u)", getpid(), seed);
        logger("srand(%d);",seed);
        srand(seed);
    }
    else
    {
        //logger("//[PRNG Seed] (0x%08X, %u)", getpid(), seed);
        logger("srand(%d);",seed);
        srand(seed);
    }
    
    
    // Initialise system call blacklist.
    /*
    char *bsd_syscall_blacklist = blacklist;
    printf("BSD Syscall Blacklist: %s\n",bsd_syscall_blacklist);
    
    char *pt;
    int i = 0;
    pt = strtok (blacklist,",");
    while (pt != NULL) {
        int a = atoi(pt);
        printf("%d\n", a);
        bsdcall_blacklist[i++] = a;
        pt = strtok (NULL, ",");
    }
    */
    
    // Populate our object store, i.e. using default types.
    make_OBJECTS();
    //populate fd db
    make_fd();
    
    // Start the fuzzing
    start_fuzzing();
    
    return 0;
}
