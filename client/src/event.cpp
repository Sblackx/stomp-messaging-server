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
Event::Event(const std::string &frame_body)
    : team_a_name(""), team_b_name(""), name(""), time(0),
      game_updates(), team_a_updates(), team_b_updates(), description("")
{
    // Parse the spec's plain-text wire format line by line
    std::istringstream ss(frame_body);
    std::string line;
    // 0=general, 1=team_a, 2=team_b, 3=description
    int section = -1;

    while (std::getline(ss, line))
    {
        if (line.empty())
            continue;

        if (line.rfind("team a: ", 0) == 0)
        {
            team_a_name = line.substr(8);
            continue;
        }
        if (line.rfind("team b: ", 0) == 0)
        {
            team_b_name = line.substr(8);
            continue;
        }
        if (line.rfind("event name: ", 0) == 0)
        {
            name = line.substr(12);
            continue;
        }
        if (line.rfind("time: ", 0) == 0)
        {
            time = std::stoi(line.substr(6));
            continue;
        }
        if (line == "general game updates:")
        {
            section = 0;
            continue;
        }
        if (line == "team a updates:")
        {
            section = 1;
            continue;
        }
        if (line == "team b updates:")
        {
            section = 2;
            continue;
        }
        if (line.rfind("description: ", 0) == 0)
        {
            description = line.substr(13);
            section = 3;
            continue;
        }

        // key: value lines inside sections
        size_t colon = line.find(": ");
        if (colon != std::string::npos)
        {
            std::string key = line.substr(0, colon);
            std::string val = line.substr(colon + 2);
            if (section == 0)
                game_updates[key] = val;
            else if (section == 1)
                team_a_updates[key] = val;
            else if (section == 2)
                team_b_updates[key] = val;
        }
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
