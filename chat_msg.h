#ifndef CHAT_MSG_H
#define CHAT_MSG_H

#include "structHeader.h"

#include <iostream>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

//message { header, body }

class chat_message
{
public:
    enum
    {
        header_length = sizeof(Header)
    };
    enum
    {
        max_body_length = 512
    };

    chat_message() {}

    const char *data() const
    {
        return data_;
    }

    char *data()
    {
        return data_;
    }

    std::size_t length() const
    {
        return header_length + m_header.bodySize;
    }

    const char *body() const
    {
        return data_ + header_length;
    }

    char *body()
    {
        return data_ + header_length;
    }

    int type() const
    {
        return m_header.type;
    }

    std::size_t body_length() const
    {
        return m_header.bodySize;
    }

    void setMessage(int messageType, const void *buffer, size_t bufferSize)
    {
        assert(bufferSize <= max_body_length);
        m_header.bodySize = bufferSize;
        m_header.type = messageType;
        std::memcpy(body(), buffer, bufferSize);
        std::memcpy(data(), &m_header, sizeof(m_header));
    }

    bool decode_header()
    {
        std::memcpy(data(), &m_header, header_length);
        if (m_header.bodySize > max_body_length)
        {
            std::cout << "body size " << m_header.bodySize << " " << m_header.type << std::endl;
            return false;
        }
        return true;
    }

private:
    char data_[header_length + max_body_length];
    Header m_header;
};

//parse message
bool parseMessage(const std::string &input, int *type, std::string &outbuffer)
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
            *type = 1;
        BindName bindInfo;
        bindInfo.nameLen = name.size();
        std::memcpy(&(bindInfo.name), name.data(), name.size());
        auto buffer = reinterpret_cast<const char *>(&bindInfo);
        outbuffer.assign(buffer, buffer + sizeof(bindInfo));
        return true;
    }
    else if (command == "Chat")
    {
        std::string chat = input.substr(pos + 1);
        if (chat.size() > 256)
            return false;
        ChatInfomation info;
        info.infoLen = chat.size();
        std::memcpy(&(info.information), chat.data(), chat.size());
        auto buffer = reinterpret_cast<const char *>(&info);
        outbuffer.assign(buffer, buffer + sizeof(info));
        if (type)
            *type = 2;
        return true;
    }
    return false;
}
//--------------------------------------------------------------------

#endif