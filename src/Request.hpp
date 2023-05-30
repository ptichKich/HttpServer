#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>

class Request {
public:
    Request(int clientfd) {
        this->clientfd = clientfd;
    }

    void Handle() {
        //TODO: implement handle
    }

private:
    int clientfd;
};

