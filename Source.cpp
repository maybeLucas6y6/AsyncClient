#include "Client.hpp"
#include <iostream>
#include <string>

int main()
{
    char address[] = "127.0.0.1";
    char port[] = "3000";
    Client cln(address, port);

    // suspend main thread execution
    std::string s;
    while (getline(std::cin, s)) {
        cln.RegisterMessage(s);
    }
}