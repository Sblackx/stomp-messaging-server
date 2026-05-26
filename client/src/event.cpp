#include "../include/event.h"
#include "../include/json.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include "event.h"
using json = nlohmann::json;

Event::Event(std::string team_a_name, std::string team_b_name, std::string name, int time,
             std::map<std::string, std::string> game_updates, std::map<std::string, std::string> team_a_updates,
             std::map<std::string, std::string> team_b_updates, std::string discription)
    : team_a_name(team_a_name), team_b_name(team_b_name), name(name),
      time(time), game_updates(game_updates), team_a_updates(team_a_updates),
      team_b_updates(team_b_updates), description(discription)
{
}

Event::~Event()
{
}

const std::string &Event::get_team_a_name() const
{
    return this->team_a_name;
}

const std::string &Event::get_team_b_name() const
{
    return this->team_b_name;
}

const std::string &Event::get_name() const
{
    return this->name;
}

int Event::get_time() const
{
    return this->time;
}

const std::map<std::string, std::string> &Event::get_game_updates() const
{
    return this->game_updates;
}

const std::map<std::string, std::string> &Event::get_team_a_updates() const
{
    return this->team_a_updates;
}

const std::map<std::string, std::string> &Event::get_team_b_updates() const
{
    return this->team_b_updates;
}

const std::string &Event::get_discription() const
{
    return this->description;
}

// std::string Event::event_to_string() const
// { // i thought this will be send, but it must be a json format str
//     // team a: usa
//     // team b: canada
//     // event name: kickoff
//     // time: 0
//     // general game updates:
//     // active: true
//     // before halftime: true
//     // team a updates:
//     // team b updates:
//     // description:
//     // The game has started! What an exciting evening!
//     //     <team_a_name> vs <team_b_name>
//     // Game stats:
//     // General stats:
//     // <stat_name1 >: <stat_val1>
//     // <stat_name2 >: <stat_val2>
//     // ...
//     // <team_a_name> stats:
//     // <stat_name1 >: <stat_val1>
//     // <stat_name2 >: <stat_val2>
//     // ...
//     // <team_b_name> stats:
//     // <stat_name1 >: <stat_val1>
//     // <stat_name2 >: <stat_val2>
//     // ...
//     // Game event reports:
//     // <game_event_time1>- <game_event_name1>:
//     // <game_event_description1>
//     // <game_event_time2>- <game_event_name2>:
//     // <game_event_description2>
//     std::string name_a = get_team_a_name();
//     std::string name_b = get_team_b_name();

//     std::string res = (name_a + " vs " + name_b + "\n");
//     res += "Game stats:\n";
//     res += "General stats:\n";
//     for (auto &up : get_game_updates())
//     {
//         res += (up.first + ": " + up.second + "\n");
//     }
//     // res += ("team b: " + get_team_b_name() + "\n");
//     // res += ("event name: " + get_name() + "\n");
//     // res += ("general game updates:\n");

//     res += (name_a + " stats:\n");
//     for (auto &up : get_team_a_updates())
//     {
//         res += (up.first + ": " + up.second + "\n");
//     }
//     res += (name_b + " stats:\n");
//     for (auto &up : get_team_b_updates())
//     {
//         res += (up.first + ": " + up.second + "\n");
//     }
//     res += ("time: " + std::to_string(get_time()) + "-" + get_name() + "\n");

//     res += "description:\n";
//     res += get_discription();

//     return res;
// }
std::string Event::event_to_json() const
{

    json js;
    js["team a"] = get_team_a_name();
    js["team b"] = get_team_b_name();

    js["event name"] = get_name();
    js["time"] = get_time();
    js["general game updates"] = get_game_updates();
    js["team a updates"] = get_team_a_updates();
    js["team b updates"] = get_team_b_updates();
    js["description"] = get_discription();

    return js.dump();
}
Event::Event(const std::string &frame_body) : team_a_name(""), team_b_name(""), name(""), time(0), game_updates(), team_a_updates(), team_b_updates(), description("")
{ // parse a json body
    json get_data = json::parse(frame_body);
    team_a_name = get_data["team a"];
    team_b_name = get_data["team b"];
    name = get_data["event name"];
    time = get_data["time"];
    description = get_data["description"];
    // iterate over the updates
    for (auto &game_up : get_data["general game updates"].items())
    {
        std::string up;
        if (game_up.value().is_string())
            up = game_up.value();
        else
            up = game_up.value().dump();
        game_updates[game_up.key()] = up;
    }
    for (auto &game_up : get_data["team a updates"].items())
    {
        std::string up;
        if (game_up.value().is_string())
            up = game_up.value();
        else
            up = game_up.value().dump();
        team_a_updates[game_up.key()] = up;
    }
    for (auto &game_up : get_data["team b updates"].items())
    {
        std::string up;
        if (game_up.value().is_string())
            up = game_up.value();
        else
            up = game_up.value().dump();
        team_b_updates[game_up.key()] = up;
    }
}

names_and_events parseEventsFile(std::string json_path)
{
    std::ifstream f(json_path);
    json data = json::parse(f);

    std::string team_a_name = data["team a"];
    std::string team_b_name = data["team b"];

    // run over all the events and convert them to Event objects
    std::vector<Event> events;
    for (auto &event : data["events"])
    {
        std::string name = event["event name"];
        int time = event["time"];
        std::string description = event["description"];
        std::map<std::string, std::string> game_updates;
        std::map<std::string, std::string> team_a_updates;
        std::map<std::string, std::string> team_b_updates;
        for (auto &update : event["general game updates"].items())
        {
            if (update.value().is_string())
                game_updates[update.key()] = update.value();
            else
                game_updates[update.key()] = update.value().dump();
        }

        for (auto &update : event["team a updates"].items())
        {
            if (update.value().is_string())
                team_a_updates[update.key()] = update.value();
            else
                team_a_updates[update.key()] = update.value().dump();
        }

        for (auto &update : event["team b updates"].items())
        {
            if (update.value().is_string())
                team_b_updates[update.key()] = update.value();
            else
                team_b_updates[update.key()] = update.value().dump();
        }

        events.push_back(Event(team_a_name, team_b_name, name, time, game_updates, team_a_updates, team_b_updates, description));
    }
    names_and_events events_and_names{team_a_name, team_b_name, events};

    return events_and_names;
}
