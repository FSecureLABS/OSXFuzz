//
//  logger.c
//  OSXFuzz
//

#ifndef logger_h
#define logger_h

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/syscall.h>

// Global variables used by the logger.

int disk_logging = 0;           // Set this flag to enable disk logging.
int network_logging = 0;        // Set this flag to enable network logging.

int debug = 0;                  // Flag to determine if STDOUT is used for debugging.
int debug_thread = 0;           // Set this flag to debug thread id and process ids

// Disk logging variables
char logger_path[PATH_MAX];
char logfilename[PATH_MAX];

int flag = 0;
int tcp_flag = 0;

// Network logging variables
const char *logging_host;           // The remote host to send the log data to.
int logging_port;                   // The remote port to send the log data to.

int sockfd, portno;
struct hostent *server;
struct sockaddr_in serv_addr;

unsigned int count = 0;

void disk_logger(char *data)
{
    FILE *stream = NULL;
    
    // Set the log file name
    if (!flag)
    {
        // Logger path should be specifed from cmd line.
        sprintf(logfilename,"%s/log.%ld.%d.c",logger_path,time(NULL),getpid());
        flag = 1;
    }
    
    // Open the log file.
    stream = fopen(logfilename,"a+");
    
    if (!stream)
    {
        printf("++ Could not open log file ++");
        exit(EXIT_FAILURE);
    }
    
    fprintf(stream,"%s\n",data);
    
    int fd = fileno(stream);
    
    // Flush the fd to make sure the write hits disk
    fcntl(fd,F_FULLFSYNC);
    
    fflush(stream);
    fclose(stream);
}


void network_logger(char *data)
{
    ssize_t n = 0;
    
    if (!tcp_flag) //first time the connection has been setup
    {
        //We need to setup the network
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0){
            printf("ERROR opening socket - exiting!");
            exit(0);
        }
        server = gethostbyname(logging_host);
        if (server == NULL) {
            fprintf(stderr,"ERROR, no such host\n");
            exit(0);
        }
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr,
              (char *)&serv_addr.sin_addr.s_addr,
              server->h_length);
        serv_addr.sin_port = htons(logging_port);
        if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        {
            printf("ERROR connecting");
            exit(0);
        }
        
        tcp_flag = 1;
    }
    
    //int n = write(sockfd,fmt,strlen(fmt));
    // Write the data over the TCP socket
    write(sockfd,data,1024);
    
    char buffer[256];
    bzero(buffer,256);
    
    // Read the ACK from the socket - blocking.
    n = read(sockfd,buffer,1);
    if (n < 0)
    {
        printf("ERROR reading from socket");
        exit(0);
    }
}


void logger(const char *fmt, ...)
{
    
    va_list args;
    va_start(args, fmt);
    
    char temp[1024] = {0};      // Log statement buffer
    char temp2[1024] = {0};     // PID / TID buffer
    
    vsnprintf(temp,sizeof(temp),fmt,args);
    
    if (debug_thread) {
        pid_t pid = getpid();
        unsigned long threadid = syscall(SYS_thread_selfid);
        snprintf(temp2, sizeof(temp2), "//PID: %d - TID: %lx",pid,threadid);
    }
    
    if (debug) {
        if (debug_thread) printf("%s\n",temp2);
        
        printf("%s\n",temp);
        fflush(stdout);
    }
    
    if (network_logging)
    {
        if (debug_thread) network_logger(temp2);
        network_logger(temp);
    }
    
    if (disk_logging)
    {
        if (debug_thread) disk_logger(temp2);
        disk_logger(temp);
    }
    
    va_end(args);
}

unsigned int get_time_in_ms()
{
    //struct timeval time = {0};
    
    //gettimeofday(&time,NULL);

    //unsigned long millis = (time.tv_sec * 1000) + (time.tv_sec / 1000);

    return count++;
}

void return_logger(char *func_name,unsigned int return_code)
{
    logger("//Func:%s returned: %d",func_name,return_code);
}

#endif

