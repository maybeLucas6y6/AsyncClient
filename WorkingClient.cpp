#include <asio.hpp>
#include <asio/experimental/as_tuple.hpp>
#include <iostream>

asio::awaitable<void> transfer(asio::ip::tcp::socket socket) {
    while (true) {
        char data[6] = { 0 };
        auto [e1, r1] = co_await asio::async_write(socket, asio::buffer("67890"), asio::experimental::as_tuple(asio::use_awaitable));

        auto [e2, r2] = co_await asio::async_read(socket, asio::buffer(data, 5), asio::experimental::as_tuple(asio::use_awaitable));
        if (!e2 && r2 > 0) {
            std::cout << data;
        }
    }
}

asio::awaitable<void> connect(asio::ip::basic_resolver_results<asio::ip::tcp>& res, asio::ip::tcp::socket socket) {

    auto [e, r] = co_await asio::async_connect(socket, res, asio::experimental::as_tuple(asio::use_awaitable));
    if (!e) {
        co_await transfer(std::move(socket));
    }
}

int main()
{
    char address[] = "127.0.0.1";
    char port[] = "3000";
    asio::io_context ctx;
    asio::ip::tcp::socket socket{ ctx };
    auto endpoints = asio::ip::tcp::resolver(ctx).resolve(address, port);
    asio::co_spawn(ctx, connect(endpoints, std::move(socket)), asio::detached);
    ctx.run();
}