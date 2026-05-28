#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <sstream>
#include <chrono>

#include "../include/ConnectionHandler.h"
#include "../include/StompProtocol.h"

static std::vector<std::string> SplitbySpace(const std::string &line);
static bool splitHostPort(const std::string &hostPort, std::string &hostOut, short &portOut);

int main(int argc, char *argv[])
{
    StompProtocol protocol;
    std::string pendingLine = "";

    while (true)
    {
        std::string line;
        std::vector<std::string> tokens;

        if (!pendingLine.empty())
        {
            line = pendingLine;
            pendingLine = "";
            tokens = SplitbySpace(line);
        }
        else
        {
            // wait for login
            while (true)
            {
                if (!std::getline(std::cin, line))
                    return 0; // EOF
                tokens = SplitbySpace(line);
                if (!tokens.empty() && tokens[0] == "login")
                    break;
                std::cout << "The User Is Not Logged in" << std::endl;
            }
        }

        // parse host:port and connect
        if (tokens.size() < 4)
        {
            std::cout << "Usage: login host:port user pass" << std::endl;
            continue;
        }

        std::string hostPort = tokens[1];
        std::string host;
        short port = 0;

        if (!splitHostPort(hostPort, host, port))
        {
            std::cout << "Could not connect to server" << std::endl;
            continue;
        }

        // create a NEW ConnectionHandler for each login
		//to solve conflict, reattempt 5 times 
		//we want to keep this socket alive, be reconnect fast!!!!
        
        ConnectionHandler *handler = nullptr;
        int attempt = 0, max_attempt = 5;
        bool connected = false;
        while (attempt < max_attempt && !connected)
        {
            handler = new ConnectionHandler(host, port);
            if (handler->connect())
            {
                connected = true;
                std::cerr << "Connected Successfully to the server" << std::endl;
            }
            else
            {
                delete handler;
                handler = nullptr;
                attempt++;
                if (attempt < max_attempt)
                {
                    int wait_time = attempt * 100;
                    std::cerr << "Retry to connect to the server in: " << wait_time << "ms" << std::endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
                }
            }
        }

        if (!connected)
        {
            std::cerr << "Could not connect to server" << std::endl;
            continue;
        }

        // send CONNECT frame
        {
            std::vector<std::string> frames = protocol.handleKeyboardLine(line);
            bool loginOk = true;
            for (const std::string &f : frames)
            {
                if (!handler->sendFrameAscii(f, '\0'))
                {
                    std::cout << "Could not send login frame" << std::endl;
                    handler->close();
                    delete handler;
                    handler = nullptr;
                    loginOk = false;
                    break;
                }
            }
            if (!loginOk)
            {
                protocol.resetSession();
                continue;
            }
        }

        // start threads for this session
        std::atomic<bool> running{true};

        std::thread serverResponseThread([&]()
        {
            std::string frame;
            while (running)
            {
                frame.clear();
                if (!handler->getFrameAscii(frame, '\0'))
                {
                    running = false;
                    break;
                }
                protocol.handleServerFrame(frame);
                if (protocol.shouldStop())
                {
                    running = false;
                    handler->close(); // unblock keyboard thread's next send
                    break;
                }
            }
        });

        std::thread keyboardParserThread([&]()
        {
            std::string cmd;
            while (running && std::getline(std::cin, cmd))
            {
                if (!running)
                {
                    pendingLine = cmd; // save line consumed after logout
                    break;
                }
                std::vector<std::string> frames = protocol.handleKeyboardLine(cmd);
                for (const std::string &f : frames)
                {
                    if (!handler->sendFrameAscii(f, '\0'))
                    {
                        running = false;
                        break;
                    }
                }
                if (protocol.shouldStop())
                {
                    running = false;
                    break;
                }
            }
            running = false;
        });

        if (keyboardParserThread.joinable())
            keyboardParserThread.join();

        if (handler != nullptr)
            handler->close();

        if (serverResponseThread.joinable())
            serverResponseThread.join();

        if (handler != nullptr)
        {
            delete handler;
            handler = nullptr;
        }

        protocol.resetSession();
    }

    return 0;
}

// helper functions
static std::vector<std::string> SplitbySpace(const std::string &line)
{
    std::istringstream iss(line);
    std::vector<std::string> tokens;
    std::string word;
    while (iss >> word)
        tokens.push_back(word);
    return tokens;
}

static bool splitHostPort(const std::string &hostPort, std::string &hostOut, short &portOut)
{
    size_t pos = hostPort.find(':');
    if (pos == std::string::npos)
        return false;
    hostOut = hostPort.substr(0, pos);
    portOut = static_cast<short>(std::stoi(hostPort.substr(pos + 1)));
    return true;
}
