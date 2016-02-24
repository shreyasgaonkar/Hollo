#include "main_window.h"
#include "chat_client.hpp"

#include <QApplication>
#include <thread>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <boost/asio.hpp>


#include "/home/matt/school/cs440/Code/server/user.hpp"


chat_client *clientp;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::resolver resolver(io_service);
    auto endpoint_iterator = resolver.resolve({ "localhost", "1337" });
    chat_client c(io_service, endpoint_iterator);
    clientp = &c;

    std::thread t([&io_service](){ io_service.run(); });

    MainWindow w;
    w.show();

    return a.exec();
}
