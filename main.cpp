#include "server.h"
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <signal.h>

bool getSettings(int argc, char ** argv, 
                 int& port_listen,
                 char *& ip_listen,
                 char *& path_root);

static void daemonize()
{
    pid_t pid = fork(); 
    if (pid < 0) { 
        fprintf(stderr, "fork error"); 
        exit(EXIT_FAILURE); 
    } 
 
    if (pid > 0) { 
        //close parent
        exit(EXIT_SUCCESS); 
    } 
    
    fprintf(stdout, "demonize sussess\n");
 
    close(STDIN_FILENO); 
    close(STDOUT_FILENO); 
    close(STDERR_FILENO);
}

int main(int argc, char **argv)
{
    int port_listen = 0;
    char *sip_listen = NULL;
    struct in_addr ip_listen;
    char *path_root = NULL;
    
    
    if (!getSettings(argc, argv,
                port_listen,
                sip_listen,
                path_root)) return 1;
    
    if (inet_aton(sip_listen, &ip_listen) == 0) {
        fprintf(stderr, "Invalid ip address\n");
        return 1;
    }
    
    if ( (chdir(path_root)) < 0) { 
        fprintf(stderr, "error change dir\n"); 
        return 1; 
    }
    
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    sigaction(SIGHUP, &sa, NULL);
    
    daemonize();
    
    Server server (port_listen, ip_listen ) ;
    server.start();
    return 0;
}
