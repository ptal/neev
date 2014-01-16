#!/bin/sh
g++ daytime_client.cpp -O3 -I../../ -I../../Boost.Endian/include -lpthread -lboost_system -std=c++11 -o client
g++ daytime_server.cpp -O3 ../../neev/basic_server.cpp -I../../ -I../../Boost.Endian/include -lpthread -lboost_system -std=c++11 -o server