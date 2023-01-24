#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma ide diagnostic ignored "misc-no-recursion"
#include "TransportNet.h"
#include <memory>
#include <iostream>
#include <limits>
#include <algorithm>
#include <tuple>
using namespace std;

shared_ptr<Junction> TransportNet::addJunction(const string& t, const string& name) {
    for (shared_ptr<Junction> junction : junctions) {
        if (junction->name == name) return junction;
    }
    shared_ptr<Junction> j(new Junction(t, name));
    junctions.push_back(j);
    return j;
}

bool TransportNet::addLine(const shared_ptr<Junction>& a, const shared_ptr<Junction>& b, int time, const string& vehicle) {
    bool exist = false;
    //checking if a route from a to b using this vehicle already exist if yes the updating the ride time if it's shorter:
    if (lines.find(a) != lines.end()) {
        exist = true;
        for (const shared_ptr<Line>& line : lines.find(a)->second) {
            if (line->start->name == a->name && line->destination->name == b->name && line->vehicle == vehicle) {
                if (line->rideTime > time) line->rideTime = time;
                return true;
            }
        }
    }

    //no line found so creating a new one:
    shared_ptr<Line> l(new Line(a, b, time, vehicle));
    if (!exist) {
        vector<shared_ptr<Line>> vec;
        vec.push_back(l);
        lines.insert({a, vec});
    } else lines.find(a)->second.push_back(l);
    return true;
}

vector<shared_ptr<Junction>> TransportNet::bfsHelper(const shared_ptr<Junction>& source, const shared_ptr<Junction>& newSource, const string& vehicle){
    vector<shared_ptr<Junction>> reachedJunctions;
    reachedJunctions.push_back(newSource);
    if (lines.find(newSource) != lines.end()){
        for (const shared_ptr<Line>& line : lines.find(newSource)->second) {
            if (line->destination->name != source->name) {
                if (line->vehicle == vehicle) {
                    for (const shared_ptr<Junction>& j: bfsHelper(source, line->destination, vehicle)) {
                        reachedJunctions.push_back(j);
                    }
                }
            }
        }
    }
    return reachedJunctions;
}

void TransportNet::bfs(const shared_ptr<Junction> &source) {
    map<string, vector<shared_ptr<Junction>>> destinations = {{"bus", vector<shared_ptr<Junction>>()}, {"tram", vector<shared_ptr<Junction>>()}, {"sprinter", vector<shared_ptr<Junction>>()}, {"rail", vector<shared_ptr<Junction>>()}};
    auto it = destinations.begin();
    //loop through all destinations of the source:
    if (lines.find(source) != lines.end()) {
        for (const shared_ptr<Line>& line : lines.find(source)->second) {
            //for each destination bfs on the destination
            for (const shared_ptr<Junction>& j : bfsHelper(source, line->destination, line->vehicle)) {
                destinations.find(line->vehicle)->second.push_back(j);
            }
        }
    }
    while (it != destinations.end()) {
        cout << it->first << ": ";
        if (it->second.empty()) cout << "no outbound travel";
        else {
            for (const shared_ptr<Junction>& j : it->second) {
                cout << j->name << "\t";
            }
        }
        cout << endl;
        it++;
    }

}

set<shared_ptr<Junction>> TransportNet::bfsReverseHelper(const string& target, const shared_ptr<Junction> &newTarget, const string& vehicle) {
    set<shared_ptr<Junction>> reachedJunctions;
    reachedJunctions.insert(newTarget);
    for (const shared_ptr<Junction>& j : junctions) {
        if (j->name != target) {
            if (lines.find(j) != lines.end()) {
                for (const shared_ptr<Line>& line: lines.find(j)->second) {
                    if (!line->visited) {
                        if (line->destination->name == newTarget->name && line->vehicle == vehicle) {
                            line->visited = true;
                            for (const shared_ptr<Junction>& x: bfsReverseHelper(target, line->start, vehicle)) {
                                reachedJunctions.insert(x);
                            }
                        }
                    }
                }
            }
        }
    }
    return reachedJunctions;
}

bool TransportNet::bfsReverse(const string& target) {
    map<string, vector<shared_ptr<Junction>>> sources = {{"bus", vector<shared_ptr<Junction>>()}, {"tram", vector<shared_ptr<Junction>>()}, {"sprinter", vector<shared_ptr<Junction>>()}, {"rail", vector<shared_ptr<Junction>>()}};
    auto it = sources.begin();
    shared_ptr<Junction> targetJunction = nullptr;
    //loop through all junctions:
    for (const shared_ptr<Junction>& j : junctions) {
        if (j->name != target) {
            if (lines.find(j) != lines.end()) {
                //loop through the destinations and check if target is one of them:
                for (const shared_ptr<Line>& line: lines.find(j)->second) {
                    if (!line->visited) {
                    //if yes, entering to bfs reverse helper with the start junction
                        if (line->destination->name == target) {
                            line->visited = true;
                            for (const shared_ptr<Junction>& x: bfsReverseHelper(target, line->start, line->vehicle)) {
                                sources.find(line->vehicle)->second.push_back(x);
                            }
                        }
                    }
                }
            }
        } else targetJunction = j;
    }
    if (targetJunction == nullptr) {
        cerr << target << " does not exist in the current network." << endl;
        return false;
    }
    while (it != sources.end()) {
        cout << it->first << ": ";
        if (it->second.empty()) cout << "no inbound travel";
        else {
            for (const shared_ptr<Junction>& j : it->second) {
                cout << j->name << "\t";
            }
        }
        cout << endl;
        it++;
    }
    //updating all the lines unvisited again:
    for (const shared_ptr<Junction>& j : junctions) {
        if (lines.find(j) != lines.end()) {
            for (const shared_ptr<Line>& line: lines.find(j)->second) {
                line->visited = false;
            }
        }
    }
    return true;
}

struct smaller{
    bool operator()(const pair<shared_ptr<Junction>, int>& p1, const pair<shared_ptr<Junction>, int>& p2) const {
        return p1.second > p2.second;
    }
};

int TransportNet::dijkstraBySameVehicleHelper(const shared_ptr<Junction> &source, const shared_ptr<Junction> &target ,const string& vehicle) {
    int inf = numeric_limits<int>::max();
    map<shared_ptr<Junction>, tuple< int, shared_ptr<Junction>, bool>> shortestDistanceFromSource;
    map<shared_ptr<Junction>, tuple< int, shared_ptr<Junction>, bool>>::iterator it;
    vector<shared_ptr<Junction>> visited;
    vector<pair<shared_ptr<Junction>, int>> unvisited;
    //initializing the distances of all junctions to be infinite apart from the source which is getting a zero:
    for (const shared_ptr<Junction>& j : junctions) {
        if (j == source) {
            unvisited.emplace_back(j, 0);
            tuple< int, shared_ptr<Junction>, bool> t  (0, nullptr, false);
            shortestDistanceFromSource.insert({j, t}).second;
        } else {
            unvisited.emplace_back(j, inf);
            tuple< int, shared_ptr<Junction>, bool> t  (inf, nullptr, false);
            shortestDistanceFromSource.insert({j, t}).second;
        }
    }
    while (!unvisited.empty()) {
        make_heap(unvisited.begin(), unvisited.end(), smaller());
        pair<shared_ptr<Junction>, int> current = unvisited.front();
        //loop through the neighbors of the smallest distance junction
        if (lines.find(current.first) != lines.end()){
            for (const shared_ptr<Line>& line : lines.find(current.first)->second) {
                if (line->vehicle == vehicle) {
                    if (current.first != source && current.first != target) {
                        //check if current junction's distance with source is bigger than the one we came from right now including the stop time:
                        if (get<0>(shortestDistanceFromSource.find(line->destination)->second) > (get<0>(shortestDistanceFromSource.find(current.first)->second)+line->rideTime+stopTimeTable.find(vehicle)->second)){
                            get<0>(shortestDistanceFromSource.find(line->destination)->second) = get<0>(shortestDistanceFromSource.find(current.first)->second)+line->rideTime+stopTimeTable.find(vehicle)->second;
                            get<1>(shortestDistanceFromSource.find(line->destination)->second) = current.first;
                        }
                    } else {
                        //check if current junction's distance with source is bigger than the one we came from right now without the stop time:
                        if (get<0>(shortestDistanceFromSource.find(line->destination)->second) > (get<0>(shortestDistanceFromSource.find(current.first)->second)+line->rideTime)){
                            get<0>(shortestDistanceFromSource.find(line->destination)->second) = get<0>(shortestDistanceFromSource.find(current.first)->second)+line->rideTime;
                            get<1>(shortestDistanceFromSource.find(line->destination)->second) = current.first;
                        }
                    }
                }
            }
        } else {
            if (visited.empty()) return inf;
        }
        //updating that the current junction has been visited:
        get<2>(shortestDistanceFromSource.find(current.first)->second) = true;
        //removing the current junction from the unvisited junctions vector:
        unvisited.clear();
        it = shortestDistanceFromSource.begin();
        while (it != shortestDistanceFromSource.end()) {
            if (!get<2>(it->second)) unvisited.emplace_back(it->first, get<0>(it->second));
            it++;
        }
    }
    return get<0>(shortestDistanceFromSource.find(target)->second);
}

bool TransportNet::dijkstraBySameVehicle(const string& source, const string& target, bool user) {
    auto it = stopTimeTable.begin();
    bool found = false;
    shared_ptr<Junction> sourceJunction;
    shared_ptr<Junction> targetJunction;
    for (const shared_ptr<Junction>& j: junctions) {
        if (j->name == source) sourceJunction = j;
        if (j->name == target) targetJunction = j;
    }
    if (sourceJunction == nullptr) {
        cerr << source << " does not exist in the current network." << endl;
        return false;
    }
    if (targetJunction == nullptr) {
        cerr << target << " does not exist in the current network." << endl;
        return false;
    }
    while (it != stopTimeTable.end()) {
        int inf = numeric_limits<int>::max();
        int time = dijkstraBySameVehicleHelper(sourceJunction, targetJunction, it->first);
        if (user) cout << it->first << ": ";
        if (time == inf) {
            if (user) cout << "route unavailable" << endl;
        }
        else {
            if (user) cout << time << endl;
            found = true;
        }
        it++;
    }
    return found;
}

map<shared_ptr<Junction>, tuple< int, shared_ptr<Junction>, bool, string>> TransportNet::dijkstraByMultipleHelper(const shared_ptr<Junction> &source, const shared_ptr<Junction> &target) {
    int inf = numeric_limits<int>::max();
    //shortestDistanceFromSource is a map that the key is the junction we're on currently,
    //and the value is a tuple that contains 4 things : 1.the distance from the source 2.from which junction we came 3.bool visited\unvisited
    //4.the vehicle with whom we arrived to this junction
    map<shared_ptr<Junction>, tuple< int, shared_ptr<Junction>, bool, string>> shortestDistanceFromSource;
    map<shared_ptr<Junction>, tuple< int, shared_ptr<Junction>, bool, string>>::iterator it;
    vector<shared_ptr<Junction>> visited;
    vector<pair<shared_ptr<Junction>, int>> unvisited;
    //initializing the distances of all junctions to be infinite apart from the source which is getting a zero:
    for (const shared_ptr<Junction>& j : junctions) {
        if (j == source) {
            unvisited.emplace_back(j, 0);
            tuple< int, shared_ptr<Junction>, bool, string> t  (0, nullptr, false, "none");
            shortestDistanceFromSource.insert({j, t}).second;
        } else {
            unvisited.emplace_back(j, inf);
            tuple< int, shared_ptr<Junction>, bool, string> t  (inf, nullptr, false, "none");
            shortestDistanceFromSource.insert({j, t}).second;
        }
    }
    while (!unvisited.empty()) {
        make_heap(unvisited.begin(), unvisited.end(), smaller());
        pair<shared_ptr<Junction>, int> current = unvisited.front();
        //loop through the neighbors of the smallest distance junction :
        if (lines.find(current.first) != lines.end()){
            for (const shared_ptr<Line>& line : lines.find(current.first)->second) {
                if (get<3>(shortestDistanceFromSource.find(current.first)->second) == "none") { // it's the source junction
                    //check if current junction's distance with source is bigger than the one we came from right now without the stop time or transit time:
                    if (get<0>(shortestDistanceFromSource.find(line->destination)->second) > (get<0>(shortestDistanceFromSource.find(current.first)->second) + line->rideTime)) {
                        get<0>(shortestDistanceFromSource.find(line->destination)->second) = get<0>(shortestDistanceFromSource.find(current.first)->second) + line->rideTime;
                        get<1>(shortestDistanceFromSource.find(line->destination)->second) = current.first;
                        get<3>(shortestDistanceFromSource.find(line->destination)->second) = line->vehicle;
                    }
                } else if (get<3>(shortestDistanceFromSource.find(current.first)->second) == line->vehicle) { //we're switching between the same type of vehicle
                    // check if current junction's distance with source is bigger than the one we came from right now including the stop time:
                    if (get<0>(shortestDistanceFromSource.find(line->destination)->second) > (get<0>(shortestDistanceFromSource.find(current.first)->second)+line->rideTime+stopTimeTable.find(line->vehicle)->second)){
                        get<0>(shortestDistanceFromSource.find(line->destination)->second) = get<0>(shortestDistanceFromSource.find(current.first)->second)+line->rideTime+stopTimeTable.find(line->vehicle)->second;
                        get<1>(shortestDistanceFromSource.find(line->destination)->second) = current.first;
                        get<3>(shortestDistanceFromSource.find(line->destination)->second) = line->vehicle;
                    }
                } else if (get<3>(shortestDistanceFromSource.find(current.first)->second) != line->vehicle) { //we're switching between different types of vehicles
                    //check if current junction's distance with source is bigger than the one we came from right now including the transit time:
                    if (get<0>(shortestDistanceFromSource.find(line->destination)->second) > (get<0>(shortestDistanceFromSource.find(current.first)->second)+line->rideTime+transitTimeTable.find(current.first->type)->second)){
                        get<0>(shortestDistanceFromSource.find(line->destination)->second) = get<0>(shortestDistanceFromSource.find(current.first)->second)+line->rideTime+transitTimeTable.find(current.first->type)->second;
                        get<1>(shortestDistanceFromSource.find(line->destination)->second) = current.first;
                        get<3>(shortestDistanceFromSource.find(line->destination)->second) = line->vehicle;
                    }
                }
            }
        }
        //updating that the current junction has been visited:
        get<2>(shortestDistanceFromSource.find(current.first)->second) = true;
        //removing the current junction from the unvisited junctions vector:
        unvisited.clear();
        it = shortestDistanceFromSource.begin();
        while (it != shortestDistanceFromSource.end()) {
            if (!get<2>(it->second)) unvisited.emplace_back(it->first, get<0>(it->second));
            it++;
        }
    }
    return shortestDistanceFromSource;
}

bool TransportNet::dijkstraByMultiple(const string& source, const string& target) {
    //shortestDistanceFromSource is a map that the key is the junction we're on currently,
    //and the value is a tuple that contains 4 things : 1.the distance from the source 2.from which junction we came 3.bool visited\unvisited
    //4.the vehicle with whom we arrived to this junction
    map<shared_ptr<Junction>, tuple< int, shared_ptr<Junction>, bool, string>> shortestDistanceFromSource;
    shared_ptr<Junction> current;
    shared_ptr<Junction> sourceJunction = nullptr;
    shared_ptr<Junction> targetJunction = nullptr;
    for (const shared_ptr<Junction>& j: junctions) {
        if (j->name == source) sourceJunction = j;
        if (j->name == target) targetJunction = j;
    }
    if (sourceJunction == nullptr) {
    cerr << source << " does not exist in the current network." << endl;
    return false;
    }
    if (targetJunction == nullptr) {
        cerr << target << " does not exist in the current network." << endl;
        return false;
    }
    current = targetJunction;
    if (dijkstraBySameVehicle(sourceJunction->name, targetJunction->name, false)) {
        shortestDistanceFromSource = dijkstraByMultipleHelper(sourceJunction, targetJunction);
        cout << "the route starts from right to left: " << endl;
        while (current != sourceJunction) {
            cout << current->name << " <-- ";
            current = get<1>(shortestDistanceFromSource.find(current)->second);
        }
        cout << current->name << endl;
    } else {
        cout << "no available route" << endl;
    }
    return true;

}


#pragma clang diagnostic pop