#include "server.h"

void Server::create_socket() {
    sock = socket(PF_INET, SOCK_STREAM, 0); // creating IPv4 TCP socket
    if (sock < 0) {
        std::cerr << "Error in creating a socket" << std::endl;
        throw ServerException();
    }
}

void Server::set_server_address() {
    server_address.sin_family = AF_INET; // IPv4
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // listening on all interfaces
    server_address.sin_port = htons(_port_number); // listening on port _port_number
}

void Server::bind_socket() {
    if (bind(sock, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        std::cerr << "Error in bind" << std::endl;
        throw ServerException();
    }
}

void Server::set_listen() {
    if (listen(sock, SOMAXCONN) < 0) {
        std::cerr << "Error in listen" << std::endl;
        throw ServerException();
    }
}

Server::Server() : client_connected(false) {}

uint16_t Server::c_str_to_uint16_t(char *c_str) {
    uint16_t result;
    std::string result_string;
    try {
        result = static_cast<uint16_t >(std::stoul(c_str));
        result_string = std::to_string(result);
    }catch (std::exception& exc) {
         throw ConversionException();
    }

    //check if we didn't cut any characters during conversion
    if(strlen(result_string.c_str()) != strlen(c_str)) {
        throw ConversionException();
    }
    return result;
}

void Server::init_server(uint16_t port_number) {
    _port_number = port_number;

    create_socket();
    set_server_address();
    bind_socket();
    set_listen();

    signal(SIGPIPE, SIG_IGN); //ignore SIGPIPE to handle them manually with c++ exceptions
}

void Server::accept_new_client() {
    socklen_t client_address_len = sizeof(client_address);
    msg_sock = accept(sock, (struct sockaddr *) &client_address, &client_address_len);
    if(msg_sock < 0) {
        std::cerr << "Error in accept" << std::endl;
        if(errno == EBADF || errno == ECONNABORTED) {
            throw ProblemWithUserException();
        }
        else {
            throw ServerException();
        }
    }
    std::cout << "New client" << std::endl;
    client_connected = true;
}

std::string Server::read_message() {
    ssize_t len = read(msg_sock, buffer, sizeof(buffer)-1);
    if(len < 0) {
        std::cerr << "Error in read: ";
        if(errno == EBADF) {
            std::cerr << "Problem with user" << std::endl;
            throw ProblemWithUserException();
        }
        else {
            std::cerr << "Problem with server" << std::endl;
            throw ServerException();
        }
    }
    if(len == 0) {
        client_connected = false;
    }

    buffer[len] = '\0';
    return std::string(buffer);
}

void Server::write_message(const std::string &message) {
    size_t message_length = strlen(message.c_str());
    ssize_t snd_len = write(msg_sock, message.c_str(), message_length);
    if(snd_len != static_cast<ssize_t>(message_length)) {
        std::cerr << "Error in write: ";
        if(errno == EBADF || errno == ENOSPC || errno == EPIPE) {
            std::cerr << "Problem with user" << std::endl;
            throw ProblemWithUserException();
        }
        else {
            std::cerr << "Problem with server" << std::endl;
            throw ServerException();
        }
    }
}

bool Server::is_client_connected() {
    return client_connected;
}

void Server::disconnect_client() {
    if(close(msg_sock) < 0) {
        std::cerr << "Error in close:";
        if(errno == EBADF) {
            std::cerr << "Problem with user" << std::endl;
            throw ProblemWithUserException();
        }
        else {
            std::cerr << "Problem with server" << std::endl;
            throw ServerException();
        }
    }
    std::cout << "Client disconnected" << std::endl;
}

void Server::set_client_telnet() {
    static constexpr int NEGOTIATIONS_AMOUNT = 3, NEGOTIATION_BUFFER_SIZE = 10;
    static constexpr const char TELNET_NEGOTIATIONS[NEGOTIATIONS_AMOUNT][NEGOTIATION_BUFFER_SIZE] = {
            {
                    static_cast<char>(IAC),
                    static_cast<char>(DO),
                    static_cast<char>(TELOPT_LINEMODE)
            },
            {
                    static_cast<char>(IAC),
                    static_cast<char>(SB),
                    static_cast<char>(TELOPT_LINEMODE),
                    static_cast<char>(LM_MODE),
                    static_cast<char>(TELQUAL_IS),
                    static_cast<char>(IAC),
                    static_cast<char>(SE)
            },
            {
                    static_cast<char>(IAC),
                    static_cast<char>(WILL),
                    static_cast<char>(TELOPT_ECHO)
            }
    };

    for(int i = 0; i < NEGOTIATIONS_AMOUNT; i++) {
        write_message(TELNET_NEGOTIATIONS[i]);
    }
}

bool Server::is_telnet_negotiation(const std::string &message) {
    if(message.empty()) {
        return false;
    }
    auto first_byte = static_cast<unsigned char>(message[0]);
    return first_byte == static_cast<unsigned char>(IAC);
}

void Server::accept_telnet_negotiation(const std::string &negotiation) {
    static constexpr const char WONT_LINEMODE_NEGOTIATION[] = {
            static_cast<char>(IAC),
            static_cast<char>(WONT),
            static_cast<char>(TELOPT_LINEMODE),
            '\0'
    };

    if(negotiation.find(WONT_LINEMODE_NEGOTIATION) != std::string::npos) {
        disconnect_client();
    }
}