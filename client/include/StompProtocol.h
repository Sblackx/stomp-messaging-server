#pragma once

#include "ConnectionHandler.h"
#include <mutex>
#include <unordered_map>
#include "event.h"
#include <vector>

// TODO: implement the STOMP protocol
class StompProtocol
{
private:
    std::atomic<bool> stop;// when both threads should stop
    bool IsConnected;     // is the client connected to the server
    bool loggedIn ;        // Login command will only go through if the client is not already logged
    int NextSubId;            // Unique IDs for subscribtions
    int NextReceiptId;        // Unique IDs for receipts
    std::string host;
    int port;                                                                                     // the server's name and port
    std::string username;                                                                             // the clients username
    std::unordered_map<std::string, int> gameToSubId;                                                 // a mapping of IDs to subscribtions
    std::unordered_map<int, std::string> receiptActions;                                              // a mapping of IDs to requested receipts, to know when the servet send RECEIPT!
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Event>>> game_events; // game to user to event who the user x sent

    std::mutex m;
    // frame builders, helper function

    std::string buildConnectFrame(const std::vector<std::string> &vec);
    std::string buildSubscribeFrame(std::string &destination);
    std::string buildUnsubscribeFrame(const std::string &game);
    std::string buildDisconnectFrame(int receipt); // for locking methods that both threads can access
    void summary_into_file(std::string &file_name, std::string &game, std::string &user);
    std::vector<std::string> split_lines(const std::string &frame, char spCh) const;
    int toInt(const std::string);

public:
    StompProtocol();

    std::vector<std::string> handleKeyboardLine(const std::string &line);
    void handleServerFrame(const std::string &frame);
    bool shouldStop();
};