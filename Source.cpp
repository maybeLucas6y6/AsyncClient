#include "ExampleEnum.hpp"
#include "Message.hpp"
#include "Client.hpp"
#include <iostream>
//#include <Windows.h>

int main()
{
    Client cln("127.0.0.1", "3003");

    // suspend main thread execution
    while (true) {
        //Sleep(2000);
        Message<ExampleEnum> msg;
        msg << "12345";
        cln.RegisterMessage(msg);
    }
}