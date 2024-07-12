#include <stdio.h>
#include <iostream>
#include <netdb.h>
#include <cstring>
#include <memory>


void checkBadLinkFormat(const std::string & n) {
    std::string node = n;

    if (node.empty()) {
        return;
    }

    do {
        const char * host = &node[0], *port;
        size_t pos = node.find_last_of(":");

        if (pos == node.npos || pos + 1 == node.length()) {
            printf("host:port format error in %s \n", node.c_str());
            break;
        }

        node[pos] = 0;
        port = &node[pos + 1];
        struct addrinfo hints, *addrs;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = PF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;
        int p;
        char * end;
        p = strtol(port, &end, 0);

        if (p >= 65536 || p <= 0 || end != port + strlen(port)) {
            printf("check port failed in %s\n", node.c_str());
            break;
        }

        int status = getaddrinfo(host, port, &hints, &addrs);
        if (status != 0) {
            printf("getaddrinfo error:%s msg: %s\n", node.c_str(), gai_strerror(status));
            break;
        }

        freeaddrinfo(addrs);
        return;
    } while (0);
}

//g++ checkhost.cpp -o checkhost -g -O0 -std=c++11
int main(int argc, char* argv[]) {

    std::string hostname = argv[1];

    checkBadLinkFormat(hostname);

    return 0;
}