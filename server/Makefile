TARGET=my_server

my_server: server.cpp message.hpp user.hpp
	g++ -std="c++11" -o my_server server.cpp -lboost_system -lboost_thread -pthread

my_client: client.cpp message.hpp user.hpp
	g++ -std="c++11" -o my_client client.cpp -lboost_system -lboost_thread -pthread

debug: debug_client.cpp message.hpp user.hpp
	g++ -std="c++11" -o debug debug_client.cpp -lboost_system -lboost_thread -pthread

clean:
	rm -f my_server my_client debug
