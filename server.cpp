#include "server.h"
//#include  <sys/types.h>
#include <iostream>
#include <sstream>
#include <unistd.h> //close
#include <sys/epoll.h>
#include <fcntl.h>
#include <pthread.h>
#include "query.h"

void CreateResponse(const std::string& query, std::stringstream& response);
void CreateResponse400(std::stringstream& response);

/*

typedef struct in_addr {
  union {
    struct { u_char  s_b1, s_b2, s_b3, s_b4; } S_un_b;
    struct { u_short s_w1, s_w2; } S_un_w;
    u_long S_addr;
  } S_un;
} IN_ADDR, *PIN_ADDR, *LPIN_ADDR;


struct sockaddr {
	u_short	sa_family;
	char	sa_data[14];
};

struct sockaddr_in {
	short	sin_family;
	u_short	sin_port;
	struct in_addr	sin_addr;
	char	sin_zero[8];
};

   SOCKET  accept(SOCKET s,struct sockaddr *addr,int *addrlen);
   int     bind(SOCKET s,const struct sockaddr *name,int namelen);
   int     closesocket(SOCKET s);
   int     connect(SOCKET s,const struct sockaddr *name,int namelen);
   int     recv(SOCKET s, char *buf, int len, int flags);

*/

const int MAX_BUFFER_SIZE = 1024;



static bool set_nonblocking (int sfd)
{
  int flags = fcntl (sfd, F_GETFL, 0);
  if (flags == -1)
    {
      perror ("fcntl");
      return false;
    }

  flags |= O_NONBLOCK;
  
  int res = fcntl (sfd, F_SETFL, flags);
  if (res == -1)
    {
      perror ("fcntl");
      return false;
    }

  return true;
}

int epoll_settings(int socket_c){
    int efd = epoll_create1 (0);
    if (efd == -1){
         perror ("epoll_create");
         abort ();
    }
    
    struct epoll_event event;    
    event.data.fd = socket_c;
    event.events = EPOLLIN | EPOLLET;
    int res = epoll_ctl (efd, EPOLL_CTL_ADD, socket_c, &event);
    if (res == -1){
         perror ("epoll_ctl");
         abort ();
    }
    return efd; 
}


int processing_request( int socket_c,                  
                 std::string& partial){
    
    char buf[MAX_BUFFER_SIZE];
    for(;;) {
        int count = recv(socket_c, buf, MAX_BUFFER_SIZE, 0);
        if (count == -1) {
          if (errno == EAGAIN) continue;
          perror ("read");
          return STATE_ERROR_READ;
        }
        
        if (count == 0) {
            return STATE_WAIT_DATA;
        }
        buf[count]='\0';
        partial.append(buf);
        
        std::string query;
        int state = getOneQuery(partial, query);
        while(state != STATE_WAIT_DATA ){
            std::stringstream response;
            if ( state == STATE_BAD_REQUEST){
                CreateResponse400(response);
            } else{ //STATE_READY_QUERY
                CreateResponse(query, response);
            }
            
            /*result =*/ send(socket_c, 
                          response.str().c_str(),
                          response.str().length(), 0);
            
            state = getOneQuery(partial, query);
        }
          
    };
     
    return STATE_ERROR_READ;
}


void * fWorker(void * arg){
    int socket_c = reinterpret_cast<long> (arg);
    set_nonblocking(socket_c);
    
    int efd = epoll_settings(socket_c);
    struct epoll_event event;
    const int  maxevents =1;
    
    std::string partial;
    int state;
    for (;;) {
        state = processing_request(socket_c, partial);
        if ( state == STATE_ERROR_READ ) break;
           
        int n = epoll_wait (efd, &event, maxevents, -1);
        if (n == -1) {
            perror("epoll_wait");
            abort ();
        }
    }
    
    close(efd);
    close(socket_c);
    return NULL;
}

Server::Server(unsigned short port, struct in_addr ip_listen):
         port(port),ip_listen(ip_listen){}

Server::~Server(){
     stop();
}


void Server::start(){

    master_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
    if (master_s == -1) {
        std::cerr <<"Socket not created"<< std::endl;
        return;
    }

    struct sockaddr_in addr_master;
    addr_master.sin_family = AF_INET;
    addr_master.sin_port = htons(port);
    //addr_master.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    addr_master.sin_addr = ip_listen;
    
    if (bind(master_s, (struct sockaddr*)&addr_master, 
              sizeof(addr_master)) == -1) {
        std::cerr << "Socket not binded"<< std::endl;
        return;
    }

    if (listen(master_s, SOMAXCONN) == -1) return;

    std::cout <<"Start listenin at port( " << 
                 ntohs(addr_master.sin_port)  << ")" << std::endl;

    loop();
}

void Server::stop() {
    close(master_s);
    std::cout << "Server was stoped." << std::endl;
}


void Server::loop() {
   while (true){
     int client_s = accept(master_s, NULL, NULL);
     if (client_s == -1) continue ;

     pthread_t tid;
     if ( pthread_create(&tid, NULL, fWorker, (void*)client_s ) ){
           pthread_detach(tid);
     } 
   }
}

