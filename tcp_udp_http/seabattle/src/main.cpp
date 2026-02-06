#include "seabattle.h"

#include <atomic>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <string_view>

namespace net = boost::asio;
using net::ip::tcp;
using namespace std::literals;

void PrintFieldPair(const SeabattleField &left, const SeabattleField &right)
{
    auto left_pad = "  "s;
    auto delimeter = "    "s;
    std::cout << left_pad;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << delimeter;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << std::endl;
    for (size_t i = 0; i < SeabattleField::field_size; ++i)
    {
        std::cout << left_pad;
        left.PrintLine(std::cout, i);
        std::cout << delimeter;
        right.PrintLine(std::cout, i);
        std::cout << std::endl;
    }
    std::cout << left_pad;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << delimeter;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << std::endl;
}

template <size_t sz>
static std::optional<std::string> ReadExact(tcp::socket &socket)
{
    boost::array<char, sz> buf;
    boost::system::error_code ec;

    net::read(socket, net::buffer(buf), net::transfer_exactly(sz), ec);

    if (ec)
    {
        return std::nullopt;
    }
    //std::cout << "ReadExact data.size() "sv << buf.size() << std::endl;
    //std::cout << "ReadExact data "sv << buf.data() << std::endl;
    return {{buf.data(), sz}};
}

static bool WriteExact(tcp::socket &socket, std::string_view data)
{
    boost::system::error_code ec;
    //std::cout << "WriteExact data.size() "sv << data.size() << std::endl;
    //std::cout << "WriteExact data "sv << data << std::endl;
    net::write(socket, net::buffer(data), net::transfer_exactly(data.size()), ec);

    return !ec;
}

class SeabattleAgent
{
public:
    SeabattleAgent(const SeabattleField &field)
        : my_field_(field)
    {
    }

    void StartGame(tcp::socket &socket, bool my_initiative)
    {
        // TODO: реализуйте самостоятельно
        //std::cout << "Start Game"sv << std::endl;

        while (!IsGameEnded())
        {
            PrintFields();
            if (my_initiative)
            {
                boost::system::error_code ec;
                std::cout << "Your shoot:"sv;

                std::string data;
                std::getline(std::cin, data);
                if (!WriteExact(socket, data))
                {
                    std::cout << "Error sending data" << std::endl;
                }

                std::optional<std::string> result = ReadExact<1>(socket); // получаем результат выстрела
                if (result == std::nullopt)
                {
                    std::cout << "Response error" << std::endl;
                    return;
                }
                std::optional<std::pair<int, int>> parse = ParseMove(data); // получаем координаты выстрела

                const char ch = result->at(0);
                //std::cout << "Command -" << ch << std::endl;
                //std::cout << "parse->first Y " << parse->first << std::endl;
                //std::cout << "parse->second X " << parse->second << std::endl;

                switch (ch)
                {
                case '0':
                    my_initiative = !my_initiative;
                    other_field_.MarkMiss(parse->second, parse->first); //переворачиваем координаты
                    break;
                case '1':
                    other_field_.MarkHit(parse->second, parse->first);
                    break;
                case '2':
                    other_field_.MarkKill(parse->second, parse->first);
                    break;
                default:
                    std::cout << "Command SHOOT not found"sv << std::endl;
                    break;
                }
            }
            else
            {
                std::cout << "Wite shoot:"sv << std::endl;

                auto result = ReadExact<2>(socket);
                if (result == std::nullopt)
                {
                    std::cout << "Reading error" << std::endl;
                    return;
                }
                else
                {
                    std::cout << "Shoot: " << *result << std::endl;
                }

                auto parse = ParseMove(*result); // парсим выстрел от клиента
                if (parse != std::nullopt)
                {
                    auto r = my_field_.Shoot(parse->second, parse->first); // получаем результат выстрела
                    if (r == SeabattleField::ShotResult::MISS){
                        my_initiative = !my_initiative;
                    } 
                    bool ok = SendResult(socket, r);                       // отправляем результат выстрела
                    if (ok)
                    {
                        //std::cout << "Sending data"sv << std::endl;
                    }
                    else
                    {
                        std::cout << "Error sending data: " << std::endl;
                    }
                }
                else
                {
                    std::cout << "Error parse: invalid move '" << *result << "'" << std::endl;
                }
            }
            /*
            if (my_field_.IsLoser())
            {
                std::cout << "You all ships kills"sv << std::endl;
            }
            if (other_field_.IsLoser())
            {
                std::cout << "Other all ships kills"sv << std::endl;
            }
            */
        }
        std::cout << "Game over"sv << std::endl;
    }

private:
    static std::optional<std::pair<int, int>> ParseMove(const std::string_view &sv)
    {
        if (sv.size() != 2)
            return std::nullopt;

        int p1 = sv[0] - 'A', p2 = sv[1] - '1';
        
        //std::cout << "p1 "sv << p1 << std::endl;
        //std::cout << "p2 "sv << p2 << std::endl;

        if (p1 < 0 || p1 > 8)
            return std::nullopt;
        if (p2 < 0 || p2 > 8)
            return std::nullopt;

        return {{p1, p2}};
    }

    static std::string MoveToString(std::pair<int, int> move)
    {
        char buff[] = {static_cast<char>(move.first) + 'A', static_cast<char>(move.second) + '1'};
        return {buff, 2};
    }

    void PrintFields() const
    {
        PrintFieldPair(my_field_, other_field_);
    }

    bool IsGameEnded() const
    {
        return my_field_.IsLoser() || other_field_.IsLoser();
    }
    // TODO: добавьте методы по вашему желанию
    void ReadMove()
    {
    }
    void SendMove()
    {
    }

    static bool SendResult(tcp::socket &socket, SeabattleField::ShotResult r)
    {
        char buff;
        switch (r)
        {
        case SeabattleField::ShotResult::MISS:
            buff = '0';
            break;
        case SeabattleField::ShotResult::HIT:
            buff = '1';
            break;
        case SeabattleField::ShotResult::KILL:
            buff = '2';
            break;
        default:
            buff = '?';
            break; // на случай ошибки
        }
        return WriteExact(socket, std::string_view(&buff, 1));
        /*
        char buff = static_cast<char>(r);
        return WriteExact(socket, std::string_view(&buff, 1));
        */        
    }

    static bool ReadResult(tcp::socket &socket)
    {
        return true;
    }

private:
    SeabattleField my_field_;
    SeabattleField other_field_;
};

void StartServer(const SeabattleField &field, unsigned short port)
{
    SeabattleAgent agent(field);
    // TODO: реализуйте самостоятельно
    net::io_context io_context;
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));
    std::cout << "Waiting for connection..."sv << std::endl;

    boost::system::error_code ec;   // объект ec для сохранения кода ошибки
    tcp::socket socket{io_context}; // объект socket
    acceptor.accept(socket, ec);
    if (ec)
    {
        std::cout << "Can't accept connection"sv << std::endl;
        // return 1;
    }
    agent.StartGame(socket, false);
};

void StartClient(const SeabattleField &field, const std::string &ip_str, unsigned short port)
{
    SeabattleAgent agent(field);
    // TODO: реализуйте самостоятельно
    boost::system::error_code ec;
    auto endpoint = tcp::endpoint(net::ip::make_address(ip_str, ec), port);

    if (ec)
    {
        std::cout << "Wrong IP format"sv << std::endl;
        // return 1;
    }
    net::io_context io_context;
    tcp::socket socket{io_context};
    socket.connect(endpoint, ec);

    if (ec)
    {
        std::cout << "Can't connect to server"sv << std::endl;
        // return 1;
    }
    agent.StartGame(socket, true);
};

int main(int argc, const char **argv)
{
    if (argc != 3 && argc != 4)
    {
        std::cout << "Usage: program <seed> [<ip>] <port>" << std::endl;
        return 1;
    }

    std::mt19937 engine(std::stoi(argv[1]));
    SeabattleField fieldL = SeabattleField::GetRandomField(engine);

    if (argc == 3)
    {
        StartServer(fieldL, std::stoi(argv[2]));
    }
    else if (argc == 4)
    {
        StartClient(fieldL, argv[2], std::stoi(argv[3]));
    }
}