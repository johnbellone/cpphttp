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
    typedef boost::shared_ptr<ba::ip::tcp::socket> tcp_socket_sp;

    ba::io_service io_service;
    ba::ip::tcp::endpoint endpoint;
    ba::ip::tcp::acceptor acceptor;
    std::string data;
    int port;    

    void pump_accept();
    void write_handler(const boost::system::error_code& ec, std::size_t tx, tcp_socket_sp& sock);;
    void accept_handler(const boost::system::error_code& ec, tcp_socket_sp& sock);
public:
    simple_http(const std::string& message, int port);

    void run();
};

simple_http::simple_http(const std::string& message, int port)
    : io_service()
    , endpoint(ba::ip::tcp::v4(), port)
    , acceptor(io_service, endpoint)
    , data(message)
    , port(port)
{
    acceptor.listen();
}

void simple_http::run()
{
    pump_accept();
    std::cout << "Ready to rock and roll on port " << port << std::endl;
    io_service.run();
}

/*private*/ void simple_http::pump_accept()
{
    tcp_socket_sp sock(new ba::ip::tcp::socket(io_service));
    acceptor.async_accept(*sock, 
                          boost::bind(&simple_http::accept_handler, 
                                      this, 
                                      ba::placeholders::error,
                                      sock));
}

/*private*/ void simple_http::write_handler(const boost::system::error_code& ec, std::size_t tx, tcp_socket_sp& sock)
{
    if (!ec)
    {
        std::cout << "Wrote "<< tx << " bytes" << std::endl;
    }
}

/*private*/ void simple_http::accept_handler(const boost::system::error_code& ec, tcp_socket_sp& sock)
{
    if (!ec)
    {
        ba::async_write(*sock, ba::buffer(data), 
                        boost::bind(&simple_http::write_handler, 
                                    this,
                                    ba::placeholders::error,
                                    ba::placeholders::bytes_transferred,
                                    sock));
        pump_accept();
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
