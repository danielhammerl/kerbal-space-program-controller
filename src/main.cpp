#include <iostream>
#include <vector>
#include "boost/asio.hpp"
#include <memory>

std::vector<unsigned short> ports = {12800, 12801, 12802, 12803};
boost::asio::io_context io_context;
std::vector<boost::asio::ip::tcp::endpoint> endpoints;
std::vector<boost::asio::ip::tcp::acceptor> acceptors;
std::vector<boost::asio::ip::tcp::socket> sockets;

void start_accept(boost::asio::ip::tcp::acceptor& acceptor, size_t index) {
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context);

    acceptor.async_accept(*socket, [socket, &acceptor, &index](const boost::system::error_code& ec){
        if (!ec) {
            auto buffer = std::make_shared<std::vector<char>>(1024);
            socket->async_read_some(boost::asio::buffer(*buffer),
                [socket, buffer](const boost::system::error_code& ec, std::size_t bytes_transferred){
                    if (!ec) {
                        std::cout << "Daten empfangen: " << bytes_transferred << " Bytes\n";
                    }
                }
            );
        }
        start_accept(acceptor, index);
    });
}

int main() {
    for (size_t i = 0; i < ports.size(); ++i) {
        endpoints.emplace_back(boost::asio::ip::tcp::v4(), ports[i]);
        acceptors.emplace_back(io_context, endpoints[i]);
        sockets.emplace_back(io_context);
    }

    std::cout << "KSP Controller started" << std::endl;
    std::cout << "Opened tcp ports for subsystems on: ";
    for (size_t i = 0; i < ports.size(); ++i) {
        std::cout << endpoints[i].port();
        if (i < ports.size() - 1) std::cout << ", ";
    }
    std::cout << std::endl;

    for (size_t i = 0; i < ports.size(); ++i) {
        start_accept(acceptors[i], i);
    }

    io_context.run();

    return 0;
}
