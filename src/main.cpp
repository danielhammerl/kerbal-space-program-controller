#include <iostream>
#include <vector>
#include "boost/asio.hpp"

int main() {
    std::vector<unsigned short> ports = {12800, 12801, 12802, 12803};
    std::vector<boost::asio::io_context> io_contexts(ports.size());
    std::vector<boost::asio::ip::tcp::endpoint> endpoints;
    std::vector<boost::asio::ip::tcp::acceptor> acceptors;
    std::vector<boost::asio::ip::tcp::socket> sockets;

    for (size_t i = 0; i < ports.size(); ++i) {
        endpoints.emplace_back(boost::asio::ip::tcp::v4(), ports[i]);
        acceptors.emplace_back(io_contexts[i], endpoints[i]);
        sockets.emplace_back(io_contexts[i]);
    }

    std::cout << "KSP Controller started" << std::endl;
    std::cout << "Opened tcp ports for subsystems on: ";
    for (size_t i = 0; i < ports.size(); ++i) {
        std::cout << endpoints[i].port();
        if (i < ports.size() - 1) std::cout << ", ";
    }
    std::cout << std::endl;

    /*for (size_t i = 0; i < ports.size(); ++i) {
        acceptors[i].accept(sockets[i]);
    }*/

    return 0;
}
