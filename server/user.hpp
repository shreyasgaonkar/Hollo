#ifndef USER_HPP
#define USER_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <vector>
#include <map>
#include <list>
#include <string>
#include <mutex>

#include <boost/thread.hpp>
#include <boost/atomic.hpp>



#include "message.hpp"

class contact {
	public:
		contact(int id, std::string name, int status) {
			this->id = id;
			this->name = name;
			this->status = status;
		}

		/*All the data here is public */
		int id;
		std::string name;
		int status;
		std::vector<chat_message> c_messages;
};

class user {
	public:
		enum statuses { AVAILABLE = 1,
				BUSY,
				OFFLINE,
				AWAY };

		std::vector<contact *> friends;
		std::vector<contact *> pending;
		contact my_info;

		user(std::string name, std::string password, int id) : 
					my_info(id, name, OFFLINE), friends(), connected(false) {
			this->password = password;
		}

		user(const user& u) : my_info(u.my_info.id, 
				u.my_info.name, OFFLINE), friends(), connected(false)
		{
			this->password = u.get_pass();
		}

		void enqueue_msg(chat_message m) {
			std::cout << "Pushing a message\n";
		//	message_mtx.lock();
			msg_q.push_back(m);
		//	message_mtx.unlock();	
		}

		bool verify_pass(std::string name, std::string pass) {
			if (my_info.name == name && this->password == pass)
						return true;
			return false;
		}

		void set_status(enum statuses status) {
			my_info.status = status;
		}

		//returns true if connected properly
		bool connect(void) {
			return !connected.exchange(true);
		}

		void disconnect(void) {
			connected = false;
		}

		bool is_q_empty(void) {
		//	message_mtx.lock();
			bool ret = msg_q.empty();
		//	message_mtx.unlock();	
			return ret;
		}

		chat_message get_q_first(void) {
		//	message_mtx.lock();
			chat_message ret = msg_q.front();
			msg_q.pop_front();
		//	message_mtx.unlock();	
			return ret;
		}

		void remove_friend(int id) {
			for (int i = 0; i < friends.size(); ++i) {
				if (friends[i]->id == id) {
					friends.erase(friends.begin() + i);
					return;
				}
			}
		}

		void remove_pending(int id) {
			for (int i = 0; i < pending.size(); ++i) {
				if (pending[i]->id == id) {
					pending.erase(pending.begin() + i);
					return;
				}
			}
		}

		std::string get_pass(void) const {
			return password;
		}
	private:
		std::string password;
		boost::atomic<bool> connected;
		//std::mutex message_mtx;
		std::deque<chat_message> msg_q;
};

class user_manager {
	public:
		user_manager(int x) : next_id(1), name_to_user(), users(), users_mtx(){
			//create user 0 //nice, because i can test using it
			std::string name = "Admin";
			std::string pass = "admin__";
			user admin(name, pass, 0);
			users.push_back(admin);
			name_to_user[name] = 0;
		}

		//fail to create = -1 | success = new id
		int create_new_user (std::string name, std::string pass) {
			if (name_to_user.count(name) != 0) return -1;
			users_mtx.lock();
			int ret = next_id.fetch_add(1);
			user new_user(name, pass, ret);
			users.push_back(new_user);
			std::cout << "id: " << ret << "v.size: " << users.size() << std::endl;
			std::cout << "name: " << name << " Pass: " << pass << "\n";
			name_to_user[name] = ret;
			users_mtx.unlock();
			return ret;
		}

		bool user_exists(std::string name) {
			users_mtx.lock();
			int num = name_to_user.count(name);
			bool ret = (num == 1 ? true : false);
			for (int i = 0; i < users.size(); ++i) {
				std::cout << "User: " << i << " " << users[i].my_info.name
					<<std::endl;
			}
			std::cout << "Finished vector\n";
			for (auto it = name_to_user.begin();
					it !=name_to_user.end(); ++it) {
				std::cout << "User: " << it->first << " " << it->second << "\n";
			}
			std::cout << "\nBTW returning: " << ret << " count: " <<
				name_to_user.count(name) << std::endl;
			users_mtx.unlock();	
			return ret;
		}

		void enqueue_msg(std::string name, chat_message m) {
			users_mtx.lock();
			users[name_to_user[name]].enqueue_msg(m);
			users_mtx.unlock();	
		}

		void enqueue_pending(std::string name) {
			users_mtx.lock();
			chat_message msg;
			user *u = &users[name_to_user[name]];
			for (int i = 0; i < u->pending.size(); ++i) {
				msg.set_data(FRIEND, F_REQUEST, u->pending[i]->id, 0);
				msg.body_length(u->pending[i]->name.length());
				std::memcpy(msg.body(), u->pending[i]->name.c_str(),
						u->pending[i]->name.length());
				msg.encode_header();
				enqueue_msg(name, msg);
			}
			users_mtx.unlock();	
		}

		void enqueue_msg(int id, chat_message m) {
			users_mtx.lock();
			users[id].enqueue_msg(m);
			users_mtx.unlock();	
		}

		bool verify_pass(std::string name, std::string pass) {
			users_mtx.lock();
			bool ret = false;
			user *u = &users[name_to_user[name]];
			if (u) {
				ret = u->verify_pass(name, pass);
			}
			users_mtx.unlock();	
			return ret;
		}

		std::string get_name(int id) {
			users_mtx.lock();
			std::string ret = users[id].my_info.name;
			users_mtx.unlock();	
			return ret;
		}

		void set_status(std::string name, user::statuses status) {
			users_mtx.lock();
			users[name_to_user[name]].set_status(status);	
			std::cout << "User: " << name << " changed status to " <<
				status << std::endl;
			users_mtx.unlock();	
		}

		int get_status(std::string name) {
			users_mtx.lock();
			int ret = users[name_to_user[name]].my_info.status;
			users_mtx.unlock();	
			return ret;
		}

		int get_status(int id) {
			users_mtx.lock();
			int ret = users[id].my_info.status;
			users_mtx.unlock();	
			return ret;
		}

		int get_id(std::string name) {
			users_mtx.lock();
			int ret = users[name_to_user[name]].my_info.id;
			users_mtx.unlock();	
			return ret;
		}

		bool connect(std::string name) {
			users_mtx.lock();
			bool ret = users[name_to_user[name]].connect();
			users_mtx.unlock();	
			return ret;
		}

		void disconnect(std::string name) {
			users_mtx.lock();
			users[name_to_user[name]].disconnect();
			users_mtx.unlock();	
		}

		bool is_q_empty(std::string name) {
			users_mtx.lock();
			bool ret = users[name_to_user[name]].is_q_empty();
			users_mtx.unlock();	
			return ret;
		}

		chat_message get_q_first(std::string name) {
			users_mtx.lock();
			chat_message ret = users[name_to_user[name]].get_q_first();
			users_mtx.unlock();	
			return ret;
		}

		void remove_friend(std::string name, int id) {
			users_mtx.lock();
			users[name_to_user[name]].remove_friend(id);
			users_mtx.unlock();	
		}

		void remove_friend(int my_id, int id) {
			users_mtx.lock();
			users[my_id].remove_friend(id);
			users_mtx.unlock();	
		}

		void remove_pending(std::string name, int id) {
			users_mtx.lock();
			users[name_to_user[name]].remove_pending(id);
			users_mtx.unlock();	
		}

		void add_friend(std::string name, int friend_id) {
			users_mtx.lock();
			users[name_to_user[name]].friends.push_back( &users[friend_id].my_info);
			users_mtx.unlock();	
		}

                void add_friend(int me, int friend_id) {
                        users_mtx.lock();
                        users[me].friends.push_back( &users[friend_id].my_info);
                        users_mtx.unlock();
                }

		void add_pending(std::string name, std::string friend_name) {
			users_mtx.lock();
			user *u = &users[name_to_user[friend_name]];
			users[name_to_user[name]].pending.push_back(&u->my_info);
			users_mtx.unlock();	
		}

                void get_statuses(std::string name) {
                    users_mtx.lock();
                    user *u = &users[name_to_user[name]];
                    for (int i = 0; i < u->friends.size(); ++i) {
                        chat_message status_update;
                        status_update.set_data((int) STATUS,
                                        u->friends[i]->status + S_CHANGE_MIN,
                                        u->friends[i]->id, 0);
                        status_update.body_length(
                                        u->friends[i]->name.length());
                        std::memcpy(status_update.body(),
                                        u->friends[i]->name.c_str(),
                                        u->friends[i]->name.length());
                        status_update.encode_header();
                        u->enqueue_msg(status_update);
                    }
                    users_mtx.unlock();
                }

		void send_out_broadcast(std::string name, chat_message msg) {
			users_mtx.lock();
			user *u = &users[name_to_user[name]];
			for (int i = 0; i < u->friends.size(); ++i) {
				users[name_to_user[u->friends[i]->name]].enqueue_msg(msg);
			}
			users_mtx.unlock();	
		}

	private:
		boost::atomic<int> next_id;
		std::map<std::string, int> name_to_user;
		std::vector<user> users;
		std::mutex users_mtx;
};
#endif
