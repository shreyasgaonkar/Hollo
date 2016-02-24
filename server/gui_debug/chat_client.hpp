#ifndef CHAT_CLIENT_HPP
#define CHAT_CLIENT_HPP

#include <cstdlib>
#include <deque>
#include <iostream>
#include <boost/asio.hpp>

#include <QObject>
#include <QtGui>

#include "/home/matt/school/cs440/Code/server/user.hpp"
#include "/home/matt/school/cs440/Code/server/message.hpp"
#include "/home/matt/school/cs440/Code/server/definitions.h"


typedef std::deque<chat_message> chat_message_queue;

class chat_client : public QObject
{
Q_OBJECT

Q_SLOT
    void send_packet(chat_message message) {
        write(message);
    }

Q_SIGNAL
    void received_packet(int num);

public:
    user *me;

public:
  chat_client(boost::asio::io_service& io_service,
      boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
    : io_service_(io_service),
      socket_(io_service)
  {
    do_connect(endpoint_iterator);
  }
  ~chat_client() {}

  chat_message get_message(void) {
      chat_message m = messages[0];
      messages.erase(messages.begin());
      return m;
  }

  void write(const chat_message& msg)
  {
    io_service_.post(
        [this, msg]()
        {
          bool write_in_progress = !write_msgs_.empty();
          write_msgs_.push_back(msg);
          if (!write_in_progress)
          {
            do_write();
          }
        });
  }

  void close()
  {
    io_service_.post([this]() { socket_.close(); });
  }

private:
  void do_connect(boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
  {
    boost::asio::async_connect(socket_, endpoint_iterator,
        [this](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator)
        {
          if (!ec)
          {
            do_read_header();
          }
        });
  }

  void do_read_header()
  {
    boost::asio::async_read(socket_,
        boost::asio::buffer(read_msg_.data(), chat_message::header_length),
        [this](boost::system::error_code ec, std::size_t /*length*/)
        {
          if (!ec)
          {
                std::cout << "In do read header\n";
                std::memset(read_msg_.body(), 100, 0);
                read_msg_.decode_header();
            do_read_body();
          }
          else
          {
                std::cout << "Closing socket\n";
            socket_.close();
          }
        });
  }

  void handle_message(void) {
      messages.push_back(read_msg_);
      emit received_packet(1);
  }

  void do_read_body()
  {
    boost::asio::async_read(socket_,
        boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
        [this](boost::system::error_code ec, std::size_t /*length*/)
        {
          if (!ec)
          {
                std::cout << "In do_read_body\n";
                handle_message();
                std::cout << "Message handled\n";
                do_read_header();
          }
          else
          {
                std::cout << "Closing socket 2\n";
            socket_.close();
          }
        });
  }

  void do_write()
  {
    boost::asio::async_write(socket_,
        boost::asio::buffer(write_msgs_.front().data(),
          write_msgs_.front().length()),
        [this](boost::system::error_code ec, std::size_t /*length*/)
        {
          if (!ec)
          {
            write_msgs_.pop_front();
            if (!write_msgs_.empty())
            {
              do_write();
            }
          }
          else
          {
                std::cout << "closing socket because do_write\n";
            socket_.close();
          }
        });
  }

private:
  boost::asio::io_service& io_service_;
  boost::asio::ip::tcp::socket socket_;
  chat_message read_msg_;
  chat_message_queue write_msgs_;
  std::vector<chat_message> messages;
};


#endif // CHAT_CLIENT_HPP

