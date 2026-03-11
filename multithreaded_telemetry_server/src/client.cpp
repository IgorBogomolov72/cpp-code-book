#include "socket_raii.h"

#include <array>
#include <fmt/format.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <string>

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

constexpr auto MAX_RECV_BUFFER_SIZE = 256;

bool send_request(const int sock, const std::string &request) {
    size_t req_pos = 0;
    const auto req_length = request.length();

    while (req_pos < req_length) {
        if (ssize_t bytes_count = send(sock, request.c_str() + req_pos, req_length - req_pos, 0); bytes_count < 0) {
            if (EINTR == errno)
                continue;
            return false;
        } else {
            req_pos += bytes_count;
        }
    }

    return true;
}

bool recv_request(const int sock) {
    std::array<char, MAX_RECV_BUFFER_SIZE> buffer;
    while (true) {
        const auto recv_bytes = recv(sock, buffer.data(), buffer.size() - 1, 0);

        std::cout << recv_bytes << " was received..." << std::endl;

        if (recv_bytes > 0) {
            buffer[recv_bytes] = '\0';
            std::cout << "------------\n"
                      << std::string(buffer.begin(), std::next(buffer.begin(), recv_bytes)) << std::endl;
            continue;
        } else if (-1 == recv_bytes) {
            if (EINTR == errno)
                continue;
            if (0 == errno)
                break;
            if (EAGAIN == errno)
                break;
            return false;
        }

        break;
    }
    return true;
}

int main(int argc, char *argv[]) {
    try {

        AddrInfo addr("127.0.0.1", "8080", AI_PASSIVE);
        Socket socket_fd(socket(addr.Get()->ai_family, addr.Get()->ai_socktype, addr.Get()->ai_protocol));

        if (socket_fd.GetFd() < 0) {
            throw std::system_error(errno, std::system_category(), "socket");
        }

        if (connect(socket_fd.GetFd(), addr.Get()->ai_addr, addr.Get()->ai_addrlen) != 0) {
            throw std::system_error(errno, std::system_category(), "connect");
        }

        char ipstr[INET6_ADDRSTRLEN];
        int port;

        socklen_t addr_size;
        struct sockaddr_storage their_addr;
        addr_size = sizeof their_addr;
        getpeername(socket_fd.GetFd(), (struct sockaddr *)&their_addr, &addr_size);

        if (their_addr.ss_family == AF_INET) {
            struct sockaddr_in *socket_peer = (struct sockaddr_in *)&their_addr;
            port = ntohs(socket_peer->sin_port);
            inet_ntop(AF_INET, &socket_peer->sin_addr, ipstr, sizeof ipstr);
        } else {
            struct sockaddr_in6 *socket_peer = (struct sockaddr_in6 *)&their_addr;
            port = ntohs(socket_peer->sin6_port);
            inet_ntop(AF_INET6, &socket_peer->sin6_addr, ipstr, sizeof ipstr);
        }

        printf("Peer IP address: %s\n", ipstr);
        printf("Peer port: %d\n", port);

        std::string sent_message{"Hello server!"};

        if (!send_request(socket_fd.GetFd(), sent_message)) {
            throw std::system_error(errno, std::system_category(), "send");
        }

        if (!recv_request(socket_fd.GetFd())) {
            throw std::system_error(errno, std::system_category(), "recv");
        }
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Success!" << std::endl;

    return 0;
}
