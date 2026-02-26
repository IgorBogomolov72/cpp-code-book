#pragma once


#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdexcept>

class Socket {
    public:
        Socket() 
            : fd_(-1)
            {}
        explicit Socket(int fd) 
            : fd_(fd)
            {}
        ~Socket(){
            if (fd_ != -1){
                close(fd_);
            }
            std::cout << "Destructed socket" << std::endl;
        }
        Socket(const Socket&) = delete;
        Socket& operator=(const Socket&) = delete;
        Socket(Socket&& other) noexcept 
            : fd_(other.fd_)
            {
                other.fd_ = -1;
            }

        Socket& operator=(Socket&& other) noexcept {
            if (this != &other){
                if (fd_ != -1) close(fd_);
                fd_ = other.fd_;
                other.fd_ = -1;
            }
            return *this;
        }
        int GetFd() const {return fd_;}
        void Reset(int fd = -1){
            if(fd_ != -1) close(fd_);
            fd_ = fd;
        }
    private:
        int fd_;
};

class AddrInfo {
    public:
        AddrInfo(const char* host = NULL, const char* port = "8080", int flags = 0)
        {
            struct addrinfo hints;
            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags = flags;

            int status = getaddrinfo(host, port, &hints, &ai_);
            if(status !=0) {
                throw std::runtime_error(gai_strerror(status));
            }

        }
        ~AddrInfo(){
            if(ai_) {
                freeaddrinfo(ai_);
            }
            std::cout << "Destructed addrinfo" << std::endl;
        }
        AddrInfo(const AddrInfo&) = delete;
        AddrInfo& operator = (const AddrInfo&) = delete;

        struct addrinfo* Get() const {return ai_;}
        
    private:
        struct addrinfo* ai_;
};