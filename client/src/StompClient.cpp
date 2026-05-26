#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <sstream>

#include "../include/ConnectionHandler.h"
#include "../include/StompProtocol.h"

static std::vector<std::string> SplitbySpace(const std::string &line);
static bool splitHostPort(const std::string &hostPort, std::string &hostOut, short &portOut);

int main(int argc, char *argv[])
{
	StompProtocol protocol;

	// wait for a login command ( first command must be login)
	std::string line;
	std::vector<std::string> tokens;

	while (true)
	{
		if (!std::getline(std::cin, line))
		{
			return 0; // EOF
		}
		tokens = SplitbySpace(line);
		if (!tokens.empty() && tokens[0] == "login")
			break;

		std::cout << "The User Is Not Logged in" << std::endl;
	}

	// parse host:port and connect
	if (tokens.size() < 4)
	{
		// input is assumed legal but we can guard still
		std::cout << "The User Is not Logged in" << std::endl;
		return 0;
	}

	std::string hostPort = tokens[1];
	std::string host;
	short port = 0;

	if (!splitHostPort(hostPort, host, port))
	{
		std::cout << "Could not connect to server " << std::endl;
		return 0;
	}

	ConnectionHandler handler(host, port);
	if (!handler.connect())
	{
		std::cout << "Could not connect to server " << std::endl;
		return 0;
	}

	// send CONNECT frame given by protocol from the login line
	{
		std::vector<std::string> frames = protocol.handleKeyboardLine(line);
		for (const std::string &f : frames)
		{
			if (!handler.sendFrameAscii(f, '\0'))
			{
				std::cout << "Could not connect to server" << std::endl;
				handler.close();
				return 0;
			}
		}
	}

	// we start the two threads
	std::atomic<bool> running{true};
	std::mutex coutMutex;

	std::thread serverResponseThread([&]()
									 {
        std::string frame;
        while (running) {
			frame.clear();
            if (!handler.getFrameAscii(frame, '\0')) {
                running = false;
                break;
            }
			protocol.handleServerFrame(frame);//void, it's will print
            // std::string out = protocol.handleServerFrame(frame);
            // if (!out.empty()) {
            //     std::lock_guard<std::mutex> lg(coutMutex);
            //     std::cout << out << std::endl;
            // }

            if (protocol.shouldStop()) {
                running = false;
                break;
            }
        } });

	std::thread keyboardParserThread([&]()
									 {
        std::string cmd;
        while (running && std::getline(std::cin, cmd)) {
            std::vector<std::string> frames = protocol.handleKeyboardLine(cmd);

            for (const std::string& f : frames) {
                if (!handler.sendFrameAscii(f, '\0')) {
                    running = false;
                    break;
                }
            }

            if (protocol.shouldStop()) {
                running = false;
                break;
            }
        }
        running = false; });

	if (keyboardParserThread.joinable())
		keyboardParserThread.join();
	if (serverResponseThread.joinable())
		serverResponseThread.join();

	handler.close();
	return 0;
}

// helper functions
static std::vector<std::string> SplitbySpace(const std::string &line)
{
	std::istringstream iss(line);
	std::vector<std::string> tokens;
	std::string word;
	while (iss >> word)
	{
		tokens.push_back(word);
	}
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