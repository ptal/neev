#!/bin/sh
g++ daytime_client.cpp -O3 -I../../include/ -I../../Boost.Endian/include -lpthread -lboost_system -std=c++11 -o client
g++ daytime_server.cpp -O3 ../../include/neev/basic_server.cpp -I../../include -I../../Boost.Endian/include -lpthread -lboost_system -std=c++11 -o server