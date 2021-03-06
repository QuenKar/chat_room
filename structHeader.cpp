#include "structHeader.h"
#include "JsonObject.h"

#include <string>
#include <cstdlib>
#include <iostream>
//parse message

bool parseMessage2(const std::string &input, int *type, std::string &outbuffer)
{
    auto pos = input.find_first_of(" ");
    if (pos == std::string::npos)
        return false;

    if (pos == 0)
        return false;
    auto command = input.substr(0, pos);

    if (command == "BindName")
    {
        std::string name = input.substr(pos + 1);
        if (name.size() > 32)
            return false;
        if (type)
            *type = MT_BIND_NAME;
        ptree tree;
        tree.put("name", name);
        outbuffer = ptreeToJsonString(tree);
        return true;
    }
    else if (command == "Chat")
    {
        std::string chat = input.substr(pos + 1);
        if (chat.size() > 256)
            return false;
        if (type)
            *type = MT_CHAT_INFO;
        ptree tree;
        tree.put("information", chat);
        outbuffer = ptreeToJsonString(tree);
        return true;
    }
    return false;
}