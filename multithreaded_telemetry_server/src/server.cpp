#include "socket_raii.h"

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
#include <signal.h>
#include <atomic>

std::atomic<bool> stop_flag{false};

void HandleSignal(int signal)
{
    stop_flag = true;
    std::cout << " -received signal " << signal << std::endl;
}


int main(int argc, char *argv[])
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = HandleSignal;

    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGINT);
    sigaction(SIGINT, &sa, 0);

    try
    {

        AddrInfo addr(NULL, "8080", AI_PASSIVE);
        Socket socket_fd(socket(addr.Get()->ai_family, addr.Get()->ai_socktype, addr.Get()->ai_protocol));

        if (socket_fd.GetFd() < 0) {
            throw std::runtime_error("socket");
        }

        if (bind(socket_fd.GetFd(), addr.Get()->ai_addr, addr.Get()->ai_addrlen) < 0) {
            throw std::runtime_error("bind");
        }

        if (listen(socket_fd.GetFd(), SOMAXCONN) < 0) {
            throw std::runtime_error("listen");
        }

        std::cout << "Server is listening for connections..." << std::endl;

        struct sockaddr_storage their_addr;

        socklen_t addr_size = sizeof(their_addr);

        Socket client_fd(accept(socket_fd.GetFd(), (struct sockaddr *)&their_addr, &addr_size));

        if (client_fd.GetFd() < 0) {
            std::cerr << "accept error: " << strerror(errno) << std::endl;
            throw std::runtime_error("accept");
        }

        char buf[1024];

        int byte_received = recv(client_fd.GetFd(), buf, sizeof(buf), 0);

        if (byte_received <= 0) {
            std::cerr << "received error: " << strerror(errno) << std::endl;
            throw std::runtime_error("received");
        }
        
        buf[byte_received] = '\0';
        std::cout << "Received: " << buf << std::endl;

        send(client_fd.GetFd(), buf, size_t (byte_received), 0);

        std::cout << "Shutting down..." << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
