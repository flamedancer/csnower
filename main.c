/*

1. creating a socket
    #include <sys/types.h> #include <sys/socket.h>
    int socket(int domain, int type, int protocol);

     *** domains
        Include the following:
             AF_UNIX: UNIX internal (file system sockets)
             
             AF_INET: ARPA Internet protocols (UNIX network sockets)
             ...
     *** type
         The socket parameter type specifies the communication characteristics to be used for the new socket.
         Possible values include SOCK_STREAM and SOCK_DGRAM.
     *** protocol 
         The protocol used for communication is usually determined by the socket type and domain.
         There is normally no choice. The protocol parameter is used where there is a choice.
         0 selects the default proto- col, which we’ll use in all our examples
2. struct: socket Address
    Each socket domain requires its own address format.

    AF_UNIX socket_un    defind in sys/un.h
       struct sockaddr_un {
           sa_family_t sun_family; // AF_UNIX
           char sun_path[]; // pathname
       }; 

    AF_INET sockaddr_in   defind in netinet/in.h
        struct sockaddr_in {
            short int sin_family;  // AF_INET
            unsigned short in sin_port;   // Port number
            struct in_addr sin_addr;  // Inernet address
        };
        the IP address structure,in addr,is definde as follows:
        struct in_addr {
            unsigned long int s_addr;
        }
3. bind  
    The bind system call assigns the address specified in the parameter, address, to the unnamed socket associated with the file descriptor socket.
    Particular address structure pointer will need to be cast to the generic address type (struct sockaddr *) in the call to bind.
    On successful completion, bind returns 0. If it fails, it returns -1 and sets errno.
    
    #include <sys/socket.h>
    int bind(int socket, const struct sockaddr *address, size_t address_len);
4. Creating a socket queue
    To accept incoming connections on a socket, a server program must create a queue to store pending requests. 
    #include <sys/socket.h>
    int listen(int socket, int backlog);  // backlog : the maximum number of pending connections
    
5. Accept connections
    #include <sys/socket.h>
    int accept(int socket, struct sockaddr *address, size_t *address_len);

    The address of the calling client will be placed in the sockaddr structure pointed to by address. A null pointer may be used here if the client address isn’t of interest.
    The address_len parameter specifies the length of the client structure. If the client address is longer than this value, it will be truncated. Before calling accept, address_len must be set to the expected address length. On return, address_len will be set to the actual length of the calling client’s address structure.
    If there are no connections pending on the socket’s queue, accept will block (so that the program won’t continue) until a client makes a connection. You may change this behavior by using the O_NONBLOCK flag on the socket file descriptor, using the fcntl function like this:
        int flags = fcntl(socket, F_GETFL, 0); 
        fcntl(socket, F_SETFL, O_NONBLOCK|flags);

6. Host and Network Byte Ordering 
    To enable computers of different types to agree on values for multibyte integers transmitted over a net- work, you need to define a network ordering. 
    #include <netinet/in.h>
    unsigned long int htonl(unsigned long int hostlong);
    unsigned short int htons(unsigned short int hostshort);
    unsigned long int ntohl(unsigned long int netlong);
    unsigned short int ntohs(unsigned short int netshort);
        
*/




#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include "request.c"


#define READLEN 1024

int main() {
    int server_sockfd, client_sockfd;
    int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(8734);
    server_len = sizeof(server_address);
    int is_open = 1;
    if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, (int *)&is_open, sizeof(is_open)) == -1) {
        printf("setsockopt SO_REUSEADDR FAILED! ERROR[%d] ERRORINFO[%s]\n", errno, strerror(errno));
        close(server_sockfd);
        exit(1);
    }
    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);

    
    listen(server_sockfd, 5);
    while(1) {
        char ch[READLEN];
        int nread;
        printf("server  waiting\n");

        //Accept a connection
        client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd,
            (struct sockaddr *)&client_address, (socklen_t *)&client_len);
         // int flags = fcntl(client_sockfd, F_GETFL, 0);
         // fcntl(client_sockfd, F_SETFL, flags | O_NONBLOCK);
        print_readlines(client_sockfd);
        // do {
        //     
        //     nread = read(client_sockfd, &ch, READLEN);
        //     printf("%s", ch);
        //     printf("%d", nread);
        // }
        // while(nread == READLEN); 
        char response[] = "HTTP/1.1 200 OK\nContent-Type: text/html; charset=utf-8\n\n Hello World!";
        write(client_sockfd, &response, sizeof(response));
        printf("response OK!\n");
        close(client_sockfd);
    }
    close(server_sockfd);
    
    
    
}
