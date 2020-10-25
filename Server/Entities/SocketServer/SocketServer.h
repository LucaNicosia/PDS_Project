#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include "../Socket/Socket.h"

class ServerSocket: private Socket {
public:
      ServerSocket(int port);
      Socket accept(struct sockaddr_in* addr, socklen_t len);
};


#endif //SOCKETSERVER_H
