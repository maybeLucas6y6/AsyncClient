#include "ExampleEnum.hpp"
#include "ExampleStruct.hpp"
#include "Client.hpp"
#include "Message.hpp"

int main()
{
    Client<ExampleEnum> cln("6.tcp.eu.ngrok.io", "18680");

    while (!cln.IsConnected()) {
        std::cout << "Connecting...\n";
    }
    while (cln.IsConnected()) {
        Message<ExampleEnum> msg;
        msg.header.id = ExampleEnum::Three;
        ExampleStruct s{13,0};
        msg << s;
        cln.RegisterMessage(msg);
    }
}