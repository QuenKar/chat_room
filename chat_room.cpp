#include "chat_msg.h"
#include "JsonObject.h"

#include <boost/asio.hpp>

#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>

#include <cstdlib>

using boost::asio::ip::tcp;

//定义存放数据队列数据格式
using chat_message_queue = std::deque<chat_message>;

//----------------------------------------------------------------------
//chat_room
class chat_session;
using chat_session_ptr = std::shared_ptr<chat_session>;
class chat_room
{
public:
    void join(chat_session_ptr);
    void leave(chat_session_ptr);
    void deliver(const chat_message &);

private:
    std::set<chat_session_ptr> participants_;
    enum
    {
        max_recent_msgs = 100
    };
    chat_message_queue recent_msgs_;
};
//----------------------------------------------------------------------

//client 连接服务器创建 session
class chat_session : public std::enable_shared_from_this<chat_session>
{
public:
    chat_session(tcp::socket socket, chat_room &room)
        : socket_(std::move(socket)),
          room_(room)
    {
    }

    void start()
    {
        room_.join(shared_from_this());
        do_read_header();
    }

    void deliver(const chat_message &msg)
    {
        bool write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(msg);
        if (!write_in_progress)
        {
            do_write();
        }
    }

private:
    void do_read_header()
    {
        auto self(shared_from_this());
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.data(), chat_message::header_length),
                                [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                                    if (!ec && read_msg_.decode_header())
                                    {
                                        do_read_body();
                                    }
                                    else
                                    {
                                        room_.leave(shared_from_this());
                                    }
                                });
    }

    void do_read_body()
    {
        auto self(shared_from_this());
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
                                [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                                    if (!ec)
                                    {
                                        // room_.deliver(read_msg_);
                                        handleMessage();
                                        do_read_header();
                                    }
                                    else
                                    {
                                        room_.leave(shared_from_this());
                                    }
                                });
    }

    ptree toPtree()
    {
        ptree obj;
        std::stringstream ss(
            std::string(read_msg_.body(),
                        read_msg_.body() + read_msg_.body_length()));
        boost::property_tree::read_json(ss, obj);
        return obj;
    }

    void handleMessage()
    {
        if (read_msg_.type() == MT_BIND_NAME)
        {
            auto nameTree = toPtree();
            m_name = nameTree.get<std::string>("name");
        }
        else if (read_msg_.type() == MT_CHAT_INFO)
        {
            auto chat = toPtree();
            m_chatInfomation = chat.get<std::string>("information");

            auto rinfo = buildRoomInfo();
            chat_message msg;
            msg.setMessage(MT_ROOM_INFO, rinfo);
            room_.deliver(msg);
        }
        else
        {
        }
    }

    void do_write()
    {
        auto self(shared_from_this());
        boost::asio::async_write(socket_,
                                 boost::asio::buffer(write_msgs_.front().data(),
                                                     write_msgs_.front().length()),
                                 [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                                     if (!ec)
                                     {
                                         write_msgs_.pop_front();
                                         if (!write_msgs_.empty())
                                         {
                                             do_write();
                                         }
                                     }
                                     else
                                     {
                                         room_.leave(shared_from_this());
                                     }
                                 });
    }

    std::string buildRoomInfo() const
    {
        ptree tree;
        tree.put("name", m_name);
        tree.put("information", m_chatInfomation);
        return ptreeToJsonString(tree);
    }

    tcp::socket socket_;
    chat_room &room_;

    chat_message read_msg_;
    chat_message_queue write_msgs_;

    std::string m_name;
    std::string m_chatInfomation;
};

//----------------------------------------------------------------------

class chat_server
{
public:
    chat_server(boost::asio::io_context &io_context,
                const tcp::endpoint &endpoint)
        : acceptor_(io_context, endpoint)
    {
        do_accept();
    }

private:
    void do_accept()
    {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec)
                {
                    std::make_shared<chat_session>(std::move(socket), room_)->start();
                }

                do_accept();
            });
    }
    tcp::acceptor acceptor_;
    chat_room room_;
};

//----------------------------------------------------------------------

void chat_room::join(chat_session_ptr participant)
{
    participants_.insert(participant);
    for (const auto &msg : recent_msgs_)
        participant->deliver(msg);
}

void chat_room::leave(chat_session_ptr participant)
{
    participants_.erase(participant);
}

void chat_room::deliver(const chat_message &msg)
{
    recent_msgs_.push_back(msg);
    while (recent_msgs_.size() > max_recent_msgs)
        recent_msgs_.pop_front();

    for (auto &participant : participants_)
        participant->deliver(msg);
}

//----------------------------------------------------------------------

int main(int argc, char *argv[])
{
    try
    {
        if (argc < 2)
        {
            std::cerr << "Usage: chat_server <port> [<port> ...]\n";
            return 1;
        }

        boost::asio::io_context io_context;

        std::list<chat_server> servers;
        for (int i = 1; i < argc; ++i)
        {
            tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[i]));
            servers.emplace_back(io_context, endpoint);
        }

        io_context.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}