#include "ExampleEnum.hpp"
#include "Message.hpp"
#include "Client.hpp"
#include <iostream>
#include <Windows.h>

int main()
{
    Client cln("127.0.0.1", "3000");

    // suspend main thread execution
    while (true) {
        Sleep(500);
        Message<ExampleEnum> msg;
        msg.header.id = ExampleEnum::Three;
        ExampleStruct s{13,0};
        msg << s;

        //ExampleStruct x{ 0,0 };
        ////message >> s;
        //std::vector<uint8_t> v = msg.body;
        //std::memcpy(&x, v.data(), sizeof(ExampleStruct));
        //std::cout << "Contents: " << x.a << " " << x.b << "\n";

        cln.RegisterMessage(std::move(msg));
    }
}