//
// Created by giuseppetoscano on 13/06/20.
//

#ifndef LAB05_SOCKETSERVER_H
#define LAB05_SOCKETSERVER_H

#include "../Socket/Socket.h"

class ServerSocket: private Socket {
public:
      ServerSocket(int port);
      Socket accept(struct sockaddr_in* addr, socklen_t len);
};


#endif //LAB05_SOCKETSERVER_H
