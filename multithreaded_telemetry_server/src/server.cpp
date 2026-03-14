#include "device.h"
#include "socket_raii.h"
#include "thread_pool.h"

#include <algorithm>
#include <fmt/format.h>
#include <iostream>
#include <ranges>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <string_view>
#include <thread>
#include <vector>

#include <arpa/inet.h>
#include <atomic>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

std::atomic<bool> stop_flag{false};

void HandleSignal(int signal) {
    stop_flag = true;
    std::cerr << " -received signal " << signal << std::endl;
}

bool ParserData(std::string data, DeviceRegistry& device_registry) {
    std::cout << "Parser run..." << std::endl;

    auto colon_pos = data.find(':');
    if (colon_pos == std::string::npos) {
        return false;
    }

    std::string device_id = data.substr(0, colon_pos);
    std::string readings = data.substr(colon_pos + 1);

    DeviceState state;
    state.device_id_ = device_id;
    state.last_update_ = std::chrono::system_clock::now();

    std::istringstream ss(readings);
    std::string token;

    while (std::getline(ss, token, ',')) {
        auto eq_pos = token.find('=');
        if (eq_pos == std::string::npos) continue;

        std::string key = token.substr(0,eq_pos);
        double value = std::stod(token.substr(eq_pos + 1));

        if (key == "temp") {
            state.temperature_ = value;
        } else if (key == "hum") {
            state.humidity_ = value;
        } else if (key == "press") {
            state.pressure_ = value;
        }
    }

    std::cout << "Parser down." << std::endl;

    device_registry.UpdateDevice(state);

    return true;
}

void handleClient(Socket &&client_fd, DeviceRegistry& device_registry) {
    try {
        std::cout << "Connected to client." << std::endl;

        char buf[1024];

        int byte_received = recv(client_fd.GetFd(), buf, sizeof(buf), 0);

        if (byte_received < 0) {
            throw std::runtime_error("RECV");
        } else if (byte_received == 0) {
            throw std::runtime_error("Client disconnected");
        }

        buf[byte_received] = '\0';
        std::cout << "Received: " << buf << std::endl;

        if (ParserData(buf, device_registry)){
            if (send(client_fd.GetFd(), "Ok", 2, 0) < 0) {
                throw std::runtime_error("SEND");
            }
        } else {
            throw std::runtime_error("PARSER");
        }


        std::cout << "Shutting down..." << std::endl;

    } catch (const std::exception &ex) {
        std::cerr << "Exception: " << ex.what() << ", " << strerror(errno) << std::endl;
    }
}

int main(int argc, char *argv[]) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = HandleSignal;

    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGINT);
    sigaction(SIGINT, &sa, 0);

    try {

        AddrInfo addr(nullptr, "8080", AI_PASSIVE);
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

        ThreadPool pool(4);                     // Создаем пулл с 4 потоками
        DeviceRegistry device_registry;         // Создаем объект для регистрации устройств

        while (!stop_flag) {
            Socket client_fd(accept(socket_fd.GetFd(), (struct sockaddr *)&their_addr, &addr_size));

            if (client_fd.GetFd() < 0) {
                if (errno == EINTR)
                    continue;
                throw std::runtime_error("accept");
            }

            pool.enqueue([client_fd = std::move(client_fd), &device_registry]() mutable {
                handleClient(std::move(client_fd), device_registry);
            });
        }

    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << " -> " << strerror(errno) << std::endl;
        return 1;
    }

    std::cout << "Server terminates" << std::endl;

    return 0;
}
