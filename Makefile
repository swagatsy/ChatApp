all: execute

execute: client server

client: client.cpp
	g++ client.cpp -std=c++11 -o client

server: server.py
	python server.py
