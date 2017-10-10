#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class Server
{
public:
    Server(unsigned short port, struct in_addr ip_listen);
    ~Server();
    void start();
    void stop();
	
private:
    unsigned short port;
    struct in_addr ip_listen;
    int master_s;
    
    void loop();	
};
