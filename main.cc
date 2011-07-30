// jb@thunkbrightly.com - written to the music of Billy Joel
#include <iostream>
#include <string>
#include <sstream>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;
namespace ba = boost::asio;

class simple_http {
    ba::io_service io_service;
    ba::ip::tcp::endpoint endpoint;
    ba::ip::tcp::acceptor acceptor;
    ba::ip::tcp::socket sock;
    std::string data;

    void write_handler(const boost::system::error_code& ec, std::size_t tx);;
    void accept_handler(const boost::system::error_code& ec);
public:
    simple_http(const std::string& message, int port);

    void run();
};

simple_http::simple_http(const std::string& message, int port)
    : io_service()
    , endpoint(ba::ip::tcp::v4(), port)
    , acceptor(io_service, endpoint)
    , sock(io_service)
    , data(message)
{
    acceptor.listen();
    acceptor.async_accept(sock, boost::bind(&simple_http::accept_handler, this, ba::placeholders::error));

    std::cout << "Ready to rock and roll on port " << port << std::endl;;
}

void simple_http::run()
{
    io_service.run();
}

/*private*/ void simple_http::write_handler(const boost::system::error_code& ec, std::size_t tx)
{
    if (!ec)
    {
        std::cout << "Wrote "<< tx << " bytes" << std::endl;
    }
}

/*private*/ void simple_http::accept_handler(const boost::system::error_code& ec)
{
    if (!ec)
    {
        ba::async_write(sock, ba::buffer(data), 
                        boost::bind(&simple_http::write_handler, 
                                    this,
                                    ba::placeholders::error,
                                    ba::placeholders::bytes_transferred));
    }
}

int main(int argc, char* argv[]) 
{
    std::string message;
    int port;

    po::options_description desc("Test options");
    desc.add_options()
        ("port", po::value<int>(&port)->default_value(8888), "The port number to open up")
        ("message", po::value<std::string>(&message)->default_value("<html><h1>Hello world!</h1></html>"), "Message to display");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    std::ostringstream response;
    response<<"HTTP/1.0 200 OK\r\n"
            <<"Content-Type: text/html; charset=UTF-8\r\n"
            <<"Content-Length: "<<message.length()<<"\r\n"
            <<"\r\n"<<message<<"\r\n";
    message = response.str();

    try {
        simple_http service(message, port);
        service.run();
    }
    catch (const std::exception& e)
    {
        std::cerr<<e.what()<<std::endl; // spew
    }

    return 0;
}
