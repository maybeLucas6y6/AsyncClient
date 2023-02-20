#include "ExampleEnum.hpp"
#include "Message.hpp"
#include "Client.hpp"
#include <iostream>

int main()
{
    Client cln("127.0.0.1", "3000");

    // suspend main thread execution
    while (!cln.isConnected) {
        
    }
    while (cln.isConnected) {
        //std::cout << "$" << cln.messages.count() << "$";
        Sleep(5);
        Message<ExampleEnum> msg;
        msg.header.id = ExampleEnum::Three;
        ExampleStruct s{13,0};
        msg << s;
        cln.RegisterMessage(std::move(msg));
    }
}