//
// chat_server.cpp
// ~~~~~~~~~~~~~~~
//
//Edited by Mateusz Wszolek
//

#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <string>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/optional.hpp>
#include <boost/bind.hpp>

#include "user.hpp"
#include "message.hpp"
#include "definitions.h"

//----------------------------------------------------------------------

typedef std::deque<chat_message> chat_message_queue;
static int my_number = 0;

//----------------------------------------------------------------------
class my_connection {
	public:
		my_connection(void) : close(false), logged_in(false) {
			this->socket = boost::shared_ptr<boost::asio::ip::tcp::socket>(
					new boost::asio::ip::tcp::socket( this->io_service ));
			my_value = my_number++;
		}

		boost::asio::io_service io_service;
		boost::shared_ptr<boost::asio::ip::tcp::socket> socket;
		boost::asio::ip::tcp::endpoint endpoint;

		boost::shared_ptr<boost::thread> thread;

		boost::asio::io_service *master_io_service;
		int my_value;

		bool close;

		std::string my_user_name;
		user_manager *manager;
		

		//functions handle I/O
		bool handle_input(char *buffer, ssize_t bytes_read) {
			chat_message msg;
			std::memcpy(msg.data(), buffer, bytes_read);
			msg.decode_header();

			//check types
			switch (msg.get_type()) {
			case (TEXT_M):
			return send_message(msg);

			case (LOG_M):
			return log_message(msg);

			case (CREAT_ACC):
			return account_message(msg);

			case (STATUS):
			return status_message(msg);

			case (FRIEND):
			return friend_message(msg);

			default:
			std::cout << "Undefine message type: " << msg.get_type()
						<< std::endl;

			return false;
			}
			
		}

		void user_dced(void) {
			std::cout << "User DCed\n";
			manager->set_status(my_user_name, user::OFFLINE);
			send_out_status();
		}

		void handle_output(boost::asio::ip::tcp::socket &socket) {
			send_errors(socket);
			send_msgs(socket);
		}

	private:
		bool logged_in;
		std::deque<chat_message> error_msgs;

		void send_errors(boost::asio::ip::tcp::socket &socket) {
			chat_message m;
			while (!error_msgs.empty()) {
				std::cout << "Sending error message\n";
				m = error_msgs.front();
				error_msgs.pop_front();
				m.print_message();
				socket.write_some(boost::asio::buffer(m.data(), m.length()));	
			}
		}

		void enqueue_err(char * msg, ssize_t size) {
				chat_message err;
				std::memcpy(err.body(), msg, size);
				err.body_length(size);
				err.set_data(ERR, ERR, 0, 0);
				err.encode_header();
				error_msgs.push_back(err);
		}
		bool check_loggin(bool b) {
			if (!logged_in) {
				//not logged in so enqueue error message
				char err_msg[] = "USER NOT LOGGED IN";
				if (b) enqueue_err(err_msg, 18);
				return false;
			}
			return true;
		}

		void send_msgs(boost::asio::ip::tcp::socket &socket) {
			if (!check_loggin(false)) return;
			chat_message m;
			while (!manager->is_q_empty(my_user_name)) {
				std::cout << "Sending regular message\n";
				m = manager->get_q_first(my_user_name);
				m.print_message();
				socket.write_some(boost::asio::buffer(m.data(), m.length()));	
				std::cout << "================================\n";
			}
		}

		chat_message create_status_msg(void) {
			chat_message status_update;
			status_update.set_data((int) STATUS, 
					(manager->get_status(my_user_name) + S_CHANGE_MIN),
					manager->get_id(my_user_name), 0);
			status_update.body_length(
					my_user_name.length());
			std::memcpy(status_update.body(),
					my_user_name.c_str(),
					my_user_name.length());
			status_update.encode_header();
			return status_update;
		}

		void send_out_status() {
			chat_message status_update = create_status_msg();
			manager->send_out_broadcast(my_user_name, status_update);
		}

		bool send_message(chat_message msg) {
			//if user is logged in he can send a message to another one
			if (!check_loggin(true)) return true;
			//enqueue a message for another user
			manager->enqueue_msg(msg.get_to(), msg);
			return true;
		}


		bool log_message(chat_message msg) {
			char uname[16], pass[16];
			std::string sname, spass;
			chat_message success;
			switch (msg.get_code_num()) {
			case (L_IN):
			std::memcpy(uname, msg.body(), 16);
			std::memcpy(pass, msg.body() + 16, 16);
			sname = std::string(uname, 16);
			spass = std::string(pass, 16);
			my_user_name = (sname);
			if (manager->verify_pass(sname, spass) &&
						manager->connect(my_user_name)) {
				logged_in = true;
				//update my status
				manager->set_status(my_user_name, user::AVAILABLE);
				send_out_status();
				//send log_in success message
				success.set_data(LOG_M, L_SUCCESS, 0, 
						manager->get_id(my_user_name));
				success.encode_header();
                                error_msgs.push_back(success);
			} else {
				char err_msg[] = "COULDN'T LOG YOU IN";
				enqueue_err(err_msg, 19);
			}
			break;
			case (L_OUT): 
			//send messages to friends
			if (check_loggin(true)) {
				manager->set_status(my_user_name, user::OFFLINE);
                                send_out_status();
                                manager->disconnect(my_user_name);
                        }
			return false;
			default:
				std::cout << "Unknown log message: " << msg.get_code_num()
							<< std::endl;
			}
			return true;
		}


		bool account_message(chat_message msg) {
			char uname[16], pass[16];
			std::string sname, spass;
			std::memcpy(uname, msg.body(), 16);
			std::memcpy(pass, msg.body() + 16, 16);
			sname = std::string(uname, 16);
			spass = std::string(pass, 16);
			char err_m[] = "Couldn't Create user";
			chat_message reply_msg;
			if (manager->create_new_user(sname, spass) != -1) {
				std::cout << "Nes user created!\n";
				reply_msg.set_data(CREAT_ACC, C_ACC_SUCCESS, 0, 0);
				reply_msg.encode_header();
				error_msgs.push_back(reply_msg);
			} else {
				enqueue_err(err_m, 20);
			}
			return true;
		}


		bool status_message(chat_message msg) {
			if (!check_loggin(true)) return true;
			std::cout << "Status message: \n";
			enum user::statuses status;
			switch (msg.get_code_num()) {
			case (S_CHANGE_A): status = user::AVAILABLE; break;
			case (S_CHANGE_B): status = user::BUSY; break;
			case (S_CHANGE_O): status = user::OFFLINE; break;
			case (S_CHANGE_AW): status = user::AWAY; break;
                        case (S_GET_STATUSES) :
                            //TODO
                            manager->get_statuses(my_user_name);

                        break;
			default:
					std::cout << "Unknown status: " << msg.get_code_num()
								<< std::endl;
					return true;
					//TODO add error message
			}
			manager->set_status(my_user_name, status);
			send_out_status();
			return true;
		}


		bool friend_message(chat_message msg) {
			if (!check_loggin(true)) return true;
			char name[16];
			std::string sname;
			chat_message local_msg;
			switch (msg.get_code_num()) {
			case (F_REQUEST):
			std::memcpy(name, msg.body(), 16);
			std::cout << "Copied 16 bytes to the memory\n";
			sname = std::string(name, 16);
			std::cout << "User name: " << sname << std::endl;
			if (manager->user_exists(sname)) {
				std::cout << "Found a friend\n";
				//add myself to pending and send notification
				manager->add_pending(sname, my_user_name);
				local_msg.set_data(FRIEND, F_REQUEST,
						manager->get_id(my_user_name),
						manager->get_id(sname));
				local_msg.body_length(my_user_name.length());
				std::memcpy(local_msg.body(), 
						my_user_name.c_str(), 
						my_user_name.length());
				local_msg.encode_header();
				manager->enqueue_msg(sname, local_msg);
			}
			break;
			case (F_REMOVE):
			//remove from from my list, and my freind's list
			//then send him a notification that a friend has been removed
			manager->remove_friend(my_user_name, msg.get_to());
			manager->remove_friend(msg.get_to(), msg.get_from());

			manager->enqueue_msg(msg.get_to(), msg);
			break;
			case (F_ACCEPT):
			//remove from my pending list
			manager->remove_pending(my_user_name, msg.get_to());
			//then move to my friends list
			manager->add_friend(my_user_name,  msg.get_to() );
                        //add myself to friend's friend list
                        manager->add_friend(msg.get_to(), msg.get_from());
			//then send my status to the new freind
			manager->enqueue_msg (msg.get_to(), create_status_msg() );
			//get status of my new friend
			local_msg.set_data(STATUS, manager->get_status(msg.get_to())
					+ S_CHANGE_MIN, msg.get_to(), 0);
			local_msg.body_length(manager->get_name(msg.get_to()).length());
			std::memcpy(local_msg.body(),
					manager->get_name(msg.get_to()).c_str(),
					manager->get_name(msg.get_to()).length());
			local_msg.encode_header();
                        manager->enqueue_msg(my_user_name, local_msg);
			break;
			case (F_DENY):
			//remove from my pending list
			manager->remove_pending(my_user_name, msg.get_to() );
			break;
			case (F_GET_PENDING):
			manager->enqueue_pending(my_user_name);
			break;
			default:
			std::cout << "Unknown friend request: " << msg.get_code_num()
						<< std::endl;
			}
			return true;
		}
};

//----------------------------------------------------------------------
class my_server {
	public:
		my_server(
				boost::asio::io_service *io_service,
				const boost::asio::ip::tcp::endpoint &endpoint) :
			user_manage(5)	{
			this->io_service = io_service;
			this->fail = false;

			try {
				this->acceptor = new boost::asio::ip::tcp::acceptor( *io_service );
				this->acceptor->open( endpoint.protocol() );
				this->acceptor->set_option(
						boost::asio::ip::tcp::acceptor::reuse_address(true));
				this->acceptor->bind( endpoint );
				this->acceptor->listen();
			} catch ( boost::system::system_error e) {
				std::cerr << "Error binding to " << endpoint.address().to_string()
					<< ":" << endpoint.port() << " " << e.what() << std::endl;
				this->fail = true;
				return;
			}
			//if we get here it means that we had success!
			this->connection = boost::shared_ptr<my_connection>(
					new my_connection());
			this->connection->master_io_service = this->io_service;
			this->acceptor->async_accept(
					*(this->connection->socket),
					this->connection->endpoint,
					boost::bind(
						&my_server::handle_accept,
						this,
						boost::asio::placeholders::error
						)
					);
		} //end my server

		void handle_accept( const boost::system::error_code& error) {
			if ( error ) {
				std::cerr << "Acceptor failed: " << error.message() << std::endl;
				return;
			}

			std::cout << "Got connection! from: "
				<< this->connection->endpoint.address().to_string() 
				<< ":" << this->connection->endpoint.port() << std::endl;

			//create a separate thread to handle the user
			this->connection->thread = boost::shared_ptr<boost::thread>(
					new boost::thread(&my_server::worker, this, 
						this->connection));

			//now i need to rebuild the accept call
			this->connection = boost::shared_ptr<my_connection>(
					new my_connection());
			this->connection->master_io_service = this->io_service;
			this->acceptor->async_accept(
					*(this->connection->socket),
					this->connection->endpoint,
					boost::bind(
						&my_server::handle_accept,
						this,
						boost::asio::placeholders::error
						)
					);
		}

		bool fail;
	private:

		ssize_t read_withno_timeout(
				boost::asio::ip::tcp::socket &socket,
				char *buf,
				size_t count
				) {
			size_t bytes_transferred;
			if (socket.available()) {
				buf[count] = {0};
				socket.read_some(boost::asio::buffer(buf, 4));	
				bytes_transferred = std::atoi(buf);
				socket.read_some(boost::asio::buffer(buf+4, 
							bytes_transferred+chat_message::header_data));
			}
			else return 0;
			std::cout << "Read :" << bytes_transferred+4 + chat_message::header_data
				<< " bytes\n";
			return bytes_transferred+4 + chat_message::header_data;
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

		void worker(boost::shared_ptr<my_connection> connection) {
			boost::asio::ip::tcp::socket &socket = *(connection->socket);
			boost::asio::socket_base::non_blocking_io make_non_blocking( true );
			socket.io_control( make_non_blocking );

			connection->manager = &user_manage;

			char acBuffer[1024];
			std::string line("");

			std::cout << "Worker thread " << connection->my_value 
				<< " running!\n";

			while ( connection->close == false ) {
				ssize_t bytes_read = read_withno_timeout(
						socket, // socket to read
						acBuffer, // buffer to read into
						sizeof(acBuffer) // maximum size of buffer
						);

				if ( bytes_read < 0 )
					break; // connection error or close

				if ( bytes_read != 0 ) {
				//DEBUG CODE
					p_bytes(acBuffer, bytes_read);

					if (!connection->handle_input(acBuffer, bytes_read)) break;
				}
				connection->handle_output(socket);

			} // while connection not to be closed
			std::cout << "User logged out\n";
			connection->user_dced();
		}

	private:
		boost::asio::io_service *io_service;
		boost::asio::ip::tcp::endpoint endpoint;
		boost::asio::ip::tcp::acceptor *acceptor;
		boost::shared_ptr<my_connection> connection;
		user_manager user_manage;
}; //end my server object

//----------------------------------------------------------------------

int main(int argc, char* argv[])
{
	try
	{
		if (argc != 2)
		{
			std::cerr << "Usage: chat_server <port>\n";
			return 1;
		}

		boost::asio::io_service io_service;
		boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(),
				std::atoi(argv[1]));

		boost::shared_ptr<my_server> server(new my_server( &io_service, endpoint));

		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
