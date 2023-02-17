#pragma once

#include <asio.hpp>
#include <memory>
#include "MutexQueue.hpp"

class Session : public std::enable_shared_from_this<Session> {
private:
	asio::ip::tcp::socket socket;
	MutexQueue<std::string>& q;
public:
	Session(asio::ip::tcp::socket s, asio::io_context& c, MutexQueue<std::string>& mq);
	~Session();
	asio::awaitable<void> Write();
	asio::awaitable<void> Read();
	asio::awaitable<void> Transfer();
};