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
// #include <sys/epoll.h>  // linux
#include <sys/event.h>  //  mac os
#include <stdio.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <err.h> 
#include <stdlib.h>
#include <fcntl.h>
#include "request.c"
#include "poll.c"
#include "response.c"


#define READLEN 1024
#define LISTENQ 20
const int MaxEvents = 20;

int main() {
    install_handlers();

    int server_sockfd, client_sockfd;
    int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    client_len = sizeof(client_address);

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

    
    listen(server_sockfd, LISTENQ);
    // setNonBlock(server_sockfd);


    int epfd;
    int nfds;
    int sock_fd;
    struct request * this_request;
    // ** 创建 poll
    // epfd = epoll_create(256);//生成epoll句柄   linux
    if ((epfd = kqueue()) == -1)  // mac
        err(1, "Cannot create kqueue");
        
    // poll 监听 server_sock
    // ** linux 
    // struct epoll_event ev, events[MaxEvents];
    // ev.data.fd = listenfd;//设置与要处理事件相关的文件描述符 ?
    // ev.events = EPOLLIN;//设置要处理的事件类型
    // epoll_ctl(epfd, EPOLL_CTL_ADD, server_sockfd, &ev);
     
    // ** mac
    struct kevent events[MaxEvents];
    updateEvents(epfd, server_sockfd, kReadEvent, 0, NULL);

    int response_len = 1000;
    char response_default[] = "HTTP/1.1 200 OK\nContent-Type: text/html; charset=utf-8\n\n Pag Not Found!\r\n";
    char response[response_len];
    memset( response, 0, sizeof(response) );
    
    while(1) {
        //  等待
        // linux
        // nfds = epoll_wait(epfd,events,20,500);//等待事件发生
        // mac
        // struct timespec timeout;
        // timeout.tv_sec = 1000;
        // timeout.tv_nsec = 1000;
        // int n = kevent(epfd, NULL, 0, events, MaxEvents, &timeout);
        int n = kevent(epfd, NULL, 0, events, MaxEvents, NULL);
        printf("server waiting events %d \n", n);
        for(int i=0;i<n;i++) {
            // linux
            // if(events[i].data.fd == server_sockfd)  { //有新的连接
            //     //Accept a connection
            //     client_sockfd = accept(server_sockfd,
            //         (struct sockaddr *)&client_address, (socklen_t *)&client_len);
            //     printf("new connect \n");
            //     ev.data.fd = client_sockfd;
            //     ev.events = EPOLLIN;//设置监听事件为可写
            //     epoll_ctl(epfd, EPOLL_CTL_ADD, client_sockfd, &ev);//新增套接字
            // }
            // else if(events[i].events & EPOLLIN) {  //可读事件
            //     if((sock_fd = events[i].data.fd) < 0)
            //         continue;
            //     print_readlines(client_sockfd);
            //     ev.data.fd = sock_fd;
            //     ev.events = EPOLLOUT;
            //     epoll_ctl(epfd,EPOLL_CTL_MOD,sock_fd,&ev);//修改监听事件为可写
            // }
            // else if(events[i].events & EPOLLOUT) { //可写事件
            //     sock_fd = events[i].data.fd;
            //     write(sock_fd, &response, sizeof(response));
            //     printf("response OK!\n");
            //     close(sock_fd);
            // }

            // mac
            // sock_fd = (int)(intptr_t)events[i].udata;
            sock_fd = events[i].ident;
            int event_type = events[i].filter;
            printf("echo event_type %d \n", event_type);
            if (event_type == EVFILT_READ) {
                if (sock_fd == server_sockfd) {
                    //Accept a connection
                    client_sockfd = accept(server_sockfd,
                        (struct sockaddr *)&client_address, (socklen_t *)&client_len);
                    // setNonBlock(client_sockfd);
                    printf("new connect \n");
                    updateEvents(epfd, client_sockfd, kReadEvent, 0, NULL);
                } else {
                    if (sock_fd < 0)
                        continue;
                    this_request = print_readlines(sock_fd);
                    updateEvents(epfd, sock_fd, kWriteEvent, 1, (void *)(this_request));
                }
            } else if (event_type == EVFILT_WRITE) {
                this_request = (struct request *)(events[i].udata);
                // request_url = (char *)(events[i].udata);
                struct hash_item * handler_item = find_hash_item(this_request->url + 1);
                if ( NULL == handler_item ) {
                    write(sock_fd, &response_default, sizeof(response_default));
                }
                else {
                    handler_item->handler_fuc(response, response_len, this_request);
                    write(sock_fd, &response, mystrlen(response)*sizeof(char));
                }
                updateEvents(epfd, sock_fd, kReadEvent, 1, NULL);
                printf("response OK!\n");
                clear_request(this_request);
                close(sock_fd);
            } else {
                ;
            }
        }
    }
    close(server_sockfd);
    close(epfd);
    
    
    
}
