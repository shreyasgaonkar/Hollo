//
// chat_client.cpp
// ~~~~~~~~~~~~~~~
//
// Edited by Mateusz Wszolek
//

#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include "message.hpp"
#include "user.hpp"
#include "definitions.h"

using boost::asio::ip::tcp;

typedef std::deque<chat_message> chat_message_queue;
user *me;
std::string sname, spass;

class chat_client
{
public:
  chat_client(boost::asio::io_service& io_service,
      tcp::resolver::iterator endpoint_iterator)
    : io_service_(io_service),
      socket_(io_service)
  {
    do_connect(endpoint_iterator);
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
  void do_connect(tcp::resolver::iterator endpoint_iterator)
  {
    boost::asio::async_connect(socket_, endpoint_iterator,
        [this](boost::system::error_code ec, tcp::resolver::iterator)
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
	  std::cout << "Handling message\n";
	  char fname[16]; std::string fsname;
	  int accept_f;
	  contact *c;
	  read_msg_.print_message();
	  switch (read_msg_.get_type()) {
	  case (TEXT_M):
		  std::cout << "Message from: " << read_msg_.get_from()
		  << ": " << read_msg_.body() << std::endl;
	  break;
	  case (LOG_M):
	  if (read_msg_.get_code_num() == L_SUCCESS) {
			me = new user(sname, spass, read_msg_.get_to());
			std::cout << "Log in successful!\n";
	  }
	  break;
	  case (CREAT_ACC):
	  if (read_msg_.get_code_num() == C_ACC_SUCCESS) {
			std::cout << "Account cretaed successfully";
	  }
	  break;
	  case (STATUS):
			//find the friend and change the status
			for (int i=0; i < me->friends.size(); ++i) {
				if (me->friends[i]->id == read_msg_.get_from()) {
					me->friends[i]->status = read_msg_.get_code_num() - S_CHANGE_MIN;
					break;
				}
			}
	  break;
	  case (FRIEND):
	  switch (read_msg_.get_code_num()) {
		  case (F_REQUEST):
			  std::memcpy(fname, read_msg_.body(), 16);
			  fsname = std::string(fname);
				c = new contact(read_msg_.get_from(), fname, user::OFFLINE);
				std::cout << "Friend request from: " << fsname << " Accept[1]? ";
				me->pending.push_back(c);
			break;
		  case (F_REMOVE):
				for (int i = 0; i < me->friends.size(); ++i) {
					if (me->friends[i]->id == read_msg_.get_from()) {
						me->friends.erase(me->friends.begin() + i);
						break;
					}
				}
			break;
	  }
	  break;
	  case (ERR):
	  std::cout << "Error: " << read_msg_.body() << std::endl;
	  break;
	  }
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
  tcp::socket socket_;
  chat_message read_msg_;
  chat_message_queue write_msgs_;
};

void p_me(void) {
	if (!me) return;
	std::cout << "FRIENDS: \n";
	for (int i = 0; i < me->friends.size(); ++i) {
		std::cout << "name: " << me->friends[i]->name << "\tID: "
			<< me->friends[i]->id << "\tstatus: "
			<< me->friends[i]->status << std::endl;
	}
	std::cout << "PENDING: \n";
	for (int i = 0; i < me->pending.size(); ++i) {
		std::cout << "name: " << me->pending[i]->name << "\tID: "
			<< me->pending[i]->id << "\tstatus: "
			<< me->pending[i]->status << std::endl;
	}
	
}


void p_bytes(char *buffer, ssize_t num_bytes) {
	chat_message msg;
	std::memcpy(msg.data(), buffer, num_bytes);
	msg.decode_header();
	msg.print_message();
	char buf[10];
	for (int i = 0; i < num_bytes; ++i) {
		printf(" %02X", buffer[i]);
		buf[i%10] = buffer[i];

		if (i % 10 == 0) {
			for (int j=0; j < 10; j++) printf("%c", buf[j]);
			std::cout << std::endl;
		}
	}
	std::cout << std::endl;
}

void process_line(char *buffer) {
	while(*buffer) {
		if (buffer[0] == '`') buffer[0] = 0x0;
		buffer++;
	}
}

int main(int argc, char* argv[])
{
	int choice;
	std::cout << "Enter name: ";
	std::cin >> sname;
	std::cout << "Enter password: ";
	std::cin >> spass;
	try
	{
		if (argc != 3)
		{
			std::cerr << "Usage: chat_client <host> <port>\n";
			return 1;
		}

		boost::asio::io_service io_service;

		tcp::resolver resolver(io_service);
		auto endpoint_iterator = resolver.resolve({ argv[1], argv[2] });
		chat_client c(io_service, endpoint_iterator);

		std::thread t([&io_service](){ io_service.run(); });

		int type, code, from, to;
		char line[chat_message::max_body_length + 1] = {0};
		std::string sline;
		while (1)
		{
			std::cout << "[Input=1]|[Info=2]Enter: ";
			std::cin >> choice;
			if (choice == 2) {
				//TODO print output
				p_me();

				continue;
			}
			std::cout << "type: ";
			std::cin >> type;
			std::cout << "code: ";
			std::cin >> code;
			std::cout << "from ";
			std::cin >> from;
			std::cout << "to: ";
			std::cin >> to;
			
			std::cout << "[`for 0]text: \n";
			std::cin >> sline; //TODO get stuff after space
			std::cout << "TEXT: " << sline << std::endl;
			std::memcpy(line, sline.c_str(), sline.length());
			std::cout << ".length(): " << sline.length() << " strlen " <<
				std::strlen(line) << std::endl;
			chat_message msg;
			msg.body_length(sline.length());
			process_line(line);
			std::memset(msg.body(), 100, 0);

			std::memcpy(msg.body(), line, msg.body_length());
			msg.set_data(type, code, from, to);
			msg.encode_header();
			p_bytes(msg.data(), msg.length());
			c.write(msg);
		}

		c.close();
		t.join();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
