#pragma once

#include <thread>
#include <asio.hpp>
#include "MutexQueue.hpp"

class Client {
private:
	std::jthread writingThread;
	asio::io_context writingContext;
	std::jthread processingThread;
	asio::io_context processingContext;
	asio::ip::tcp::socket socket;
	asio::ip::basic_resolver_results<asio::ip::tcp> endpoints;
	MutexQueue<std::string> messages;
public:
	Client(const char* address, const char* port);
	~Client();
	asio::awaitable<void> Connect();
	asio::awaitable<void> Read();
	asio::awaitable<void> Write();
	void ProcessMessage(char data[]);
	void RegisterMessage(std::string msg);
};