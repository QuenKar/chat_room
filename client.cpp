#include "chat_msg.h"
#include "Protocol.pb.h"

#include <boost/asio.hpp>

#include <deque>
#include <iostream>
#include <thread>

#include <cstdlib>

using boost::asio::ip::tcp;

using chat_message_queue = std::deque<chat_message>;

class chat_client
{
public:
    chat_client(boost::asio::io_context &io_context,
                const tcp::resolver::results_type &endpoints)
        : io_context_(io_context),
          socket_(io_context)
    {
        do_connect(endpoints);
    }

    void write(const chat_message &msg)
    {
        //去io_context注册一个事件,完成后回调
        boost::asio::post(io_context_,
                          [this, msg]() {
                              //write_msgs_开始为空返回true
                              bool write_in_progress = !write_msgs_.empty();
                              write_msgs_.push_back(msg);
                              if (!write_in_progress)
                              {
                                  do_write();
                              }
                          });
    }

    void close()
    {
        boost::asio::post(io_context_, [this]() { socket_.close(); });
    }

private:
    void do_connect(const tcp::resolver::results_type &endpoints)
    {
        boost::asio::async_connect(socket_, endpoints,
                                   [this](boost::system::error_code ec, tcp::endpoint) {
                                       if (!ec)
                                       {
                                           do_read_header();
                                       }
                                   });
    }

    void do_read_header()
    {
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.data(), chat_message::header_length),
                                [this](boost::system::error_code ec, std::size_t /*length*/) {
                                    if (!ec && read_msg_.decode_header())
                                    {
                                        do_read_body();
                                    }
                                    else
                                    {
                                        socket_.close();
                                    }
                                });
    }

    void do_read_body()
    {
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
                                [this](boost::system::error_code ec, std::size_t /*length*/) {
                                    if (!ec)
                                    {
                                        if (read_msg_.type() == MT_ROOM_INFO)
                                        {
                                            std::string buffer(read_msg_.body(),
                                                               read_msg_.body() + read_msg_.body_length());
                                            PRoomInformation roomInfo;
                                            auto ok = roomInfo.ParseFromString(buffer);
                                            //显示服务器给各客户端返回的info
                                            if (ok)
                                            {
                                                std::cout << "client: '";
                                                std::cout << roomInfo.name();
                                                std::cout << "' says '";
                                                std::cout << roomInfo.information();
                                                std::cout << "'\n'";
                                            }
                                            else
                                            {
                                                /* 处理出错情况 */
                                            }
                                        }
                                        do_read_header();
                                    }
                                    else
                                    {
                                        socket_.close();
                                    }
                                });
    }

    void do_write()
    {
        boost::asio::async_write(socket_,
                                 boost::asio::buffer(write_msgs_.front().data(),
                                                     write_msgs_.front().length()),
                                 [this](boost::system::error_code ec, std::size_t /*length*/) {
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
                                         socket_.close();
                                     }
                                 });
    }

private:
    boost::asio::io_context &io_context_;
    tcp::socket socket_;
    chat_message read_msg_;
    chat_message_queue write_msgs_;
};

int main(int argc, char *argv[])
{
    try
    {
        //处理版本兼容性
        GOOGLE_PROTOBUF_VERIFY_VERSION;

        if (argc != 3)
        {
            std::cerr << "Usage: chat_client <host> <port>\n";
            return 1;
        }
        
        std::cout<<"bindname <your name>\n";
        std::cout<<"chat <your message>\n";

        boost::asio::io_context io_context;

        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(argv[1], argv[2]);
        chat_client c(io_context, endpoints);

        std::thread t([&io_context]() { io_context.run(); });

        char line[chat_message::max_body_length + 1];
        while (std::cin.getline(line, chat_message::max_body_length + 1))
        {
            chat_message msg;
            int type = 0;
            std::string input(line, line + std::strlen(line));
            std::string output;
            if (parseMessage(input, &type, output))
            {
                msg.setMessage(type, output.data(), output.size());
                c.write(msg);
                std::cout << "write message for server " << output.size() << std::endl;
            }
        }

        c.close();
        t.join();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    //防止检查内存泄漏的软件误操作
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}