#ifndef SIK1_SERVER_H
#define SIK1_SERVER_H

#include <iostream>
#include <netdb.h>
#include <exception>
#include <unistd.h>
#include <sstream>
#include <arpa/telnet.h>
#include <cstring>
#include <csignal>

class ServerException : public std::exception {};
class ProblemWithUserException : public std::exception {};
class ConversionException : public std::exception {};

class Server {
    static constexpr int BUFFER_SIZE = 500;

    int sock, msg_sock;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    char buffer[BUFFER_SIZE];

    uint16_t _port_number;

    bool client_connected;

    void create_socket();
    void set_server_address();
    void bind_socket();
    void set_listen();

public:
    Server();

    static uint16_t c_str_to_uint16_t(char* c_str);

    void set_client_telnet();
    void init_server(uint16_t port_number);

    void accept_new_client();

    std::string read_message();
    void write_message(const std::string& message);

    bool is_client_connected();

    void disconnect_client();

    bool is_telnet_negotiation(const std::string& message);

    void accept_telnet_negotiation(const std::string& negotiation);
};

#endif //SIK1_SERVER_H
