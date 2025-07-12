// File: ~/projects/CRFzit/daemon/cpp/main.cpp

#include "grpc_server.h"

int main(int argc, char** argv) {
    GrpcServer server;
    server.Run();
    return 0;
}