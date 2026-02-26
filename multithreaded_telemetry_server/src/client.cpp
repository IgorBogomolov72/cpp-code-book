#include <iostream>
#include <string.h>
#include <stdio.h>
#include <fmt/format.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>

int main(int argc, char* argv[]){

    struct sockaddr_storage their_addr;
    socklen_t addr_size;    
    struct addrinfo hints;
    struct addrinfo* result;
    int status;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    if ((status = getaddrinfo("127.0.0.1", "8080", &hints, &result)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(1);
    }
    
    
    int socket_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (socket_fd < 0){
        perror("client: socket");
        freeaddrinfo(result);
        return -1;
    }

    if (connect (socket_fd, result->ai_addr, result->ai_addrlen) != 0) {
        std::cerr << "connect failed: " << strerror(errno) << std::endl;
        freeaddrinfo(result);
        close(socket_fd);
        return 1;
    }

    char ipstr[INET6_ADDRSTRLEN];
    int port;

    addr_size = sizeof their_addr;
    getpeername(socket_fd, (struct sockaddr*)&their_addr, &addr_size);

    if (their_addr.ss_family == AF_INET) {
        struct sockaddr_in* socket_peer = (struct sockaddr_in*) &their_addr;
        port = ntohs(socket_peer->sin_port);
        inet_ntop(AF_INET, &socket_peer->sin_addr, ipstr, sizeof ipstr);
    } else {
        struct sockaddr_in6* socket_peer = (struct sockaddr_in6*) &their_addr;
        port = ntohs(socket_peer->sin6_port);
        inet_ntop(AF_INET6, &socket_peer->sin6_addr, ipstr, sizeof ipstr);
    }

    printf("Peer IP address: %s\n", ipstr);
    printf("Peer port: %d\n", port);

    char msg[1024] = "Hello server!";
    int len, bytes_sent;

    len = strlen(msg);

    bytes_sent = send(socket_fd, msg, len, 0);

    if (bytes_sent < 0) {
        std::cerr << "send failed: " << strerror(errno) << std::endl;
        freeaddrinfo(result);
        close(socket_fd);
        return 1;
    }

    std::cout << "Send: " << bytes_sent << " byte" << std::endl;

    char msg_recv[1024];
    recv(socket_fd, msg_recv, sizeof(msg_recv), 0);

    std::cout << "Received message: " << msg_recv << std::endl;
    std::cout << "Success" << std::endl;

    freeaddrinfo(result);
    close(socket_fd);

    return 0;
}
