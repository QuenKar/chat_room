#ifndef STRUCT_HEADER_H
#define STRUCT_HEADER_H

#include <string>

struct Header
{
    int bodySize;
    int type;
};

enum MessageType
{
    MT_BIND_NAME = 1,
    MT_CHAT_INFO = 2,
    MT_ROOM_INFO = 3,
};


bool parseMessage(const std::string &input, int *type, std::string &outbuffer);

#endif