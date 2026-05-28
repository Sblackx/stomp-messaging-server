#include "StompProtocol.h"
#include "event.h"
// #include <event.h>
#include <fstream>

StompProtocol::StompProtocol() : stop(false), IsConnected(false), loggedIn(false), NextSubId(1), NextReceiptId(1), host(), port(0), username(), gameToSubId(), receiptActions(), game_events(), m()

{
}

// server
void StompProtocol::handleServerFrame(const std::string &frame)
{
    std::vector<std::string> vec = split_lines(frame, '\n');
    
    if (vec.empty())
        return;
    int start = 0;
    while (start < vec.size() && vec.at(start).empty())
        start++;

    if (start >= vec.size())
        return;

    std::string received = vec.at(start);
    if (received == "CONNECTED")
    {
        std::cout << "Login successful" << std::endl;
        IsConnected = true;
        loggedIn = true;
    }
    else if (received == "RECEIPT")
    {
        std::vector<std::string> recId = split_lines(vec.at(start + 1), ':');
        int receipt = toInt(recId.at(1));
        std::string getAction = receiptActions[receipt]; // get the action by receipt id
        if (getAction == "logout")
        {
            stop = true;
            IsConnected = false;
            loggedIn = false;
            NextSubId = 1;
            NextReceiptId = 1;
            host.clear();
            port = 0;
            username.clear();
            gameToSubId.clear();
            receiptActions.clear();
            game_events.clear();

            std::cout << "LoginOut successful" << std::endl;
        }
        else
        {
            std::vector<std::string> subsId = split_lines(getAction, ':');
            std::string action = subsId.at(0);
            std::string des = subsId.at(1);
            if (action == "join")
            { // join the game
                gameToSubId[des] = toInt(subsId.at(2));
                std::cout << "Joined channel " << des << std::endl;
            }
            else
            { // exit

                gameToSubId.erase(des);
                receiptActions.erase(receipt);
            }
        }
    }
    else if (received == "MESSAGE")
    {
        std::string game = split_lines(vec.at(start + 3), '/').back();
        std::transform(game.begin(), game.end(), game.begin(), ::tolower);
        std::string user_line = vec.at(start + 4);
        std::string user_name = user_line.substr(6);

        size_t dest_pos = frame.find("destination:");
        size_t index = frame.find("\n\n", dest_pos);
        std::string body = frame.substr(index + 2);

        
        Event game_event(body);

        std::cout << "\n======================Game updates======================\n"
                  << std::endl;
        std::cout << "New report from " << user_name << " on " << game << ":\n";
        std::cout << "event name: " << game_event.get_name() << "\n";
        std::cout << "time: " << game_event.get_time() << "\n";
        std::cout << "general game updates:\n";
        for (auto &up : game_event.get_game_updates())
            std::cout << up.first << ": " << up.second << "\n";
        std::cout << "team a updates:\n";
        for (auto &up : game_event.get_team_a_updates())
            std::cout << up.first << ": " << up.second << "\n";
        std::cout << "team b updates:\n";
        for (auto &up : game_event.get_team_b_updates())
            std::cout << up.first << ": " << up.second << "\n";
        std::cout << "description: " << game_event.get_discription() << "\n"
                  << std::endl;
        std::cout << "\n============================================\n"
                  << std::endl;

        std::lock_guard<std::mutex> wr_lock(m);
        game_events[game][user_name].push_back(game_event);
    }
    else
    { // ERROR
        // split the meassage
        std::string msg = split_lines(vec.at(start + 1), ':').at(1);
        if (msg == "Wrong passcode")
        {
            std::cout << "Wrong password" << std::endl;
            return;
        }
        if (msg == "Already Connected")
        {
            std::cout << "User already logged in " << std::endl;
            return;
        }
    }
}

// keyboard
std::vector<std::string> StompProtocol::handleKeyboardLine(const std::string &line)
{
    std::vector<std::string> res;
    std::string frame;
    std::vector<std::string> vec = split_lines(line, ' ');
    std::string todo = vec.at(0); // need to define a lowercase method, to convert
    if (todo == "login")
    {
        if (loggedIn == true)
        {
            std::cout << "The client is already logged in, log out before trying again" << std::endl;
            return res;
        }

        frame = buildConnectFrame(vec);
        res.push_back(frame);
        return res;
    }
    if (loggedIn)
    {
        if (todo == "join")
        {
            frame = buildSubscribeFrame(vec.at(1));
            res.push_back(frame);
        }
        else if (todo == "exit")
        {
            frame = buildUnsubscribeFrame(vec.at(1));
            res.push_back(frame);
        }
        else if (todo == "logout")
        {

            frame = buildDisconnectFrame(NextReceiptId++);
            res.push_back(frame);
        }
        else if (todo == "report")
        {
           
            std::string event_path = vec.at(1);
            names_and_events parse = parseEventsFile(event_path);
            std::string game_key = parse.team_a_name + "_" + parse.team_b_name;

            std::transform(game_key.begin(), game_key.end(), game_key.begin(), ::tolower);

            std::sort(parse.events.begin(), parse.events.end(),
                      [](const Event &a, const Event &b)
                      { return a.get_time() < b.get_time(); });

            for (Event &ev : parse.events)
            {

                {
                    std::lock_guard<std::mutex> wr_lock(m);
                    game_events[game_key][username].push_back(ev);
                }

               
                std::string frame = "SEND\n";
                frame += ("destination:/" + game_key + "\n");
                frame += "\n";
                frame += ("user: " + username + "\n");
                frame += ("team a: " + parse.team_a_name + "\n");
                frame += ("team b: " + parse.team_b_name + "\n");
                frame += ("event name: " + ev.get_name() + "\n");
                frame += ("time: " + std::to_string(ev.get_time()) + "\n");
                frame += "general game updates:\n";
                for (auto &up : ev.get_game_updates())
                    frame += (up.first + ": " + up.second + "\n");
                frame += "team a updates:\n";
                for (auto &up : ev.get_team_a_updates())
                    frame += (up.first + ": " + up.second + "\n");
                frame += "team b updates:\n";
                for (auto &up : ev.get_team_b_updates())
                    frame += (up.first + ": " + up.second + "\n");
                frame += ("description: " + ev.get_discription() + "\n");


                res.push_back(frame);
            }
        }
        else
        {
            // summary
            summary_into_file(vec.at(1), vec.at(2), vec.at(3));
        }
    }
    else
    {
        printf("The client should be log in\n");
        return res;
    }

    return res;
}

std::string StompProtocol::buildConnectFrame(const std::vector<std::string> &vec)
{

    std::vector<std::string> getHostAndPort = split_lines(vec.at(1), ':');
    host = getHostAndPort.at(0);
    // get port num

    port = toInt(getHostAndPort.at(1));
    username = vec.at(2);
    std::string res = "CONNECT\n";
    res += "accept-version:1.2\n";
    res += "host:stomp.cs.bgu.ac.il\n";
    res += ("login:" + vec.at(2) + "\n");
    res += ("passcode:" + vec.at(3) + "\n\n");
    return res;
}

std::string StompProtocol::buildSubscribeFrame(std::string &destination)
{
    if (gameToSubId.count(destination) > 0)
    {
        printf("you joined this game before!");
        return {};
    }
    std::string res = "SUBSCRIBE\n";
    res += ("destination:/" + destination);
    res += ("\nid:" + std::to_string(NextSubId));
    res += ("\nreceipt:" + std::to_string(NextReceiptId) + "\n\n");
    receiptActions[NextReceiptId] = "join:" + destination + ":" + std::to_string(NextSubId);
    ++NextSubId;
    ++NextReceiptId;

    return res;
}

std::string StompProtocol::buildUnsubscribeFrame(const std::string &game)
{
    if (gameToSubId.count(game) == 0)
    {
        printf("you didnt joined this game before!");
        return {};
    }
    int id = gameToSubId[game];

    std::string res = "UNSUBSCRIBE\n";
    res += ("id:" + std::to_string(id) + "\n");
    res += ("receipt:" + std::to_string(NextReceiptId) + "\n\n");
    receiptActions[NextReceiptId] = "exit:" + game;
    ++NextReceiptId;

    return res;
}

std::string StompProtocol::buildDisconnectFrame(int receipt)
{
    std::string res = "DISCONNECT\n";

    res += ("receipt:" + std::to_string(receipt) + "\n\n");
    receiptActions[receipt] = "logout";

    return res;
}

void StompProtocol::summary_into_file(std::string &game, std::string &user, std::string &file_name)
{
    std::string game_lower = game;
    std::transform(game_lower.begin(), game_lower.end(), game_lower.begin(), ::tolower);

    std::lock_guard<std::mutex> re_lock(m);
    if (game_events.count(game_lower) == 0)
    {
        std::cerr << "game not found." << std::endl;
        return;
    }
    if (game_events[game_lower].count(user) == 0)
    {
        std::cerr << "user not found for this event." << std::endl;
        return;
    }
    std::vector<Event> events = game_events[game_lower][user];
    std::sort(events.begin(), events.end(),
              [](const Event &a, const Event &b)
              { return a.get_time() < b.get_time(); });

    std::string name = file_name + ".txt";
    std::ofstream out_file(name);
    if (!out_file.is_open())
    {
        std::cerr << "can not open file" << std::endl;
        out_file.close();
        return;
    }
    std::map<std::string, std::string> all_general_states;
    std::map<std::string, std::string> all_team_a_states;
    std::map<std::string, std::string> all_team_b_states;
    for (const auto &curr : events)
    {
        for (auto &up : curr.get_game_updates())
        {
            all_general_states[up.first] = up.second;
        }

        for (auto &up : curr.get_team_a_updates())
        {
            all_team_a_states[up.first] = up.second;
        }
        for (auto &up : curr.get_team_b_updates())
        {
            all_team_b_states[up.first] = up.second;
        }
    }

    std::string name_a = events.back().get_team_a_name();
    std::string name_b = events.back().get_team_b_name();

    std::string res = (name_a + " vs " + name_b + "\n");
    res += "Game stats:\n";
    res += "General stats:\n";
    for (auto st : all_general_states)
    {
        res += (st.first + ":" + st.second + "\n");
    }
    res += "\n";
    res += (name_a + " stats:\n");
    for (auto st : all_team_a_states)
    {
        res += (st.first + ":" + st.second + "\n");
    }
    res += "\n";
    res += (name_b + " stats:\n");
    for (auto st : all_team_b_states)
    {
        res += (st.first + ":" + st.second + "\n");
    }

    res += "\n\n";
    res += "Game event reports:\n";
    for (auto up : events)
    {
        res += (std::to_string(up.get_time()) + "-" + up.get_name() + ":\n");
        res += (up.get_discription() + "\n\n");
    }
    out_file << res;

    out_file.close();
}

// split
std::vector<std::string> StompProtocol::split_lines(const std::string &frame, char spCh) const
{
    std::vector<std::string> result;
    std::string add;
    for (char c : frame)
    {

        if (c != spCh)
        {
            add.push_back(c);
        }
        else
        {
            result.push_back(add);
            add.clear();
        }
    }
    if (!add.empty())
    {
        result.push_back(add);
        add.clear();
    }

    return result;
}
bool StompProtocol::shouldStop()
{

    return stop.load();
}
void StompProtocol::resetStop()
{
    stop = false;
}
// reset all after logout
void StompProtocol::resetSession()
{
    stop = false;
    loggedIn = false;
    IsConnected = false;

    NextSubId = 1;
    NextReceiptId = 1;

    username.clear();
    gameToSubId.clear();
    receiptActions.clear();

    //remove it so other users can call summary even if the user called report before is logged out now
    // game_events.clear(); 
}
// string to int
int StompProtocol::toInt(const std::string st)
{
    return std::stoi(st);
}
