#include <iostream>
#include "boost/asio.hpp"

int main() {
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::acceptor acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 12345));

    std::cout << "KSP Controller started" << std::endl;

    return 0;
}