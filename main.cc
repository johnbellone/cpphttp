#include <iostream>
#include <string>
#include <sstream>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

static int createSocket(int port)
{
    const int MAX_PENDING = 5;
    int sd = -1;
    try
    {
        if ((sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        {
            std::ostringstream ss;
            ss<<"Failed creating socket on port "<<port<<std::endl;
            throw std::runtime_error(ss.str());
        }

        struct sockaddr_in saddr;
        std::memset(&saddr, 0, sizeof(saddr));
        saddr.sin_family = AF_INET;
        saddr.sin_addr.s_addr = htonl(INADDR_ANY);
        saddr.sin_port = htons(port);

        if (bind(sd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0)
        {
            std::ostringstream ss;
            ss<<"Failed binding socket on port "<<port<<std::endl;
            throw std::runtime_error(ss.str());
        }

        if (listen(sd, MAX_PENDING) < 0)
        {
            std::ostringstream ss;
            ss<<"Failed listening socket on port "<<port<<std::endl;
            throw std::runtime_error(ss.str());
        }

        std::cout<<"Ready to rock and roll on port "<<port<<std::endl;
    }
    catch (const std::exception& e)
    {
        throw;
    }

    return sd;
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

    int sd = -1;

    try
    {
        sd = createSocket(port);

        struct sockaddr_in client;
        unsigned int len = sizeof(client), c_sd = -1;

        while (8)
        {
            if ((c_sd = accept(sd, (struct sockaddr*)&client, &len)) < 0)
            {
                std::ostringstream ss;
                ss<<"Failed on accept()"<<std::endl;
                throw std::runtime_error(ss.str());
            }

            int tx = message.length();
            if (send(c_sd, message.c_str(), tx, 0) != tx)
            {
                std::cerr<<"Error on sending message to "<<inet_ntoa(client.sin_addr)<<std::endl;
            }
            else
            {
                std::cout<<"Successfully sent message to "<<inet_ntoa(client.sin_addr)<<std::endl;
            }

            close(c_sd);
            c_sd = -1;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr<<e.what()<<std::endl; // spew

        if (sd > 0)
        {
            close(sd);
        }
    }

    return 0;
}
