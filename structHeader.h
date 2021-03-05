#ifndef STRUCT_HEADER_H
#define STRUCT_HEADER_H

#include <string>

struct Header
{
    int bodySize;
    int type;
};

struct BindName
{
    char name[32];
    int nameLen;
};

struct ChatInfomation
{
    char information[256];
    int infoLen;
};

struct RoomInfomation
{
    BindName name;
    ChatInfomation chat;
};

bool parseMessage(const std::string &input, int *type, std::string &outbuffer);

#endif