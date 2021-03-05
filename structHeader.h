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