#include "structHeader.h"
#include "Protocol.pb.h"

#include <string>
#include <cstdlib>
#include <iostream>
//parse message

bool parseMessage(const std::string &input, int *type, std::string &outbuffer)
{
    auto pos = input.find_first_of(" ");
    if (pos == std::string::npos)
        return false;

    if (pos == 0)
        return false;
    auto command = input.substr(0, pos);

    if (command == "bindname")
    {
        std::string name = input.substr(pos + 1);
    
        if (type)
            *type = MT_BIND_NAME;
        PBindName bindname;
        bindname.set_name(name);
        return bindname.SerializeToString(&outbuffer);
    }
    else if (command == "chat")
    {
        std::string chat = input.substr(pos + 1);
        
        if (type)
            *type = MT_CHAT_INFO;
        PChat pchat;
        pchat.set_information(chat);
        return pchat.SerializeToString(&outbuffer);
    }
    return false;
}