#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <iostream>

using namespace std::string_literals;

int main(){
    int new_socket = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, IPPROTO_UDP); //int domain, int type, int protocol
    close(new_socket);
    if(new_socket == -1){
        perror("Socket failed");
        return 1;
    }
    std::cout << "Open socket: "s << new_socket << std::endl;

    if(close(new_socket) == -1){
        perror("Close failed");
        return 1;
    }
    std::cout << "Close socket" << std::endl;

    return 0;
}