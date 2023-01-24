#ifndef NEVERLANDTRANSPORTNETWORK_TRANSPORTNET_H
#define NEVERLANDTRANSPORTNETWORK_TRANSPORTNET_H
#include <vector>
#include <map>
#include <string>
#include <memory>
#include "Line.h"
#include "Junction.h"
#include <set>
#include <utility>

using namespace std;

class TransportNet {
public:
    shared_ptr<Junction> addJunction(const string& t, const string& name);
    bool addLine(const shared_ptr<Junction>& a, const shared_ptr<Junction>& b, int time, const string& vehicle);
    void bfs(const shared_ptr<Junction>& source);
    vector<shared_ptr<Junction>> bfsHelper(const shared_ptr<Junction>& source,const shared_ptr<Junction>& newSource, const string& vehicle);
    bool bfsReverse(const string& target);
    set<shared_ptr<Junction>> bfsReverseHelper(const string& target,const shared_ptr<Junction>& newSource, const string& vehicle);
    int dijkstraBySameVehicleHelper(const shared_ptr<Junction>& source, const shared_ptr<Junction> &target, const string& vehicle);
    bool dijkstraBySameVehicle(const string& source, const string& target, bool user);
    map<shared_ptr<Junction>, tuple< int, shared_ptr<Junction>, bool, string>> dijkstraByMultipleHelper(const shared_ptr<Junction>& source, const shared_ptr<Junction> &target);
    bool dijkstraByMultiple(const string& source, const string& target);
    vector<shared_ptr<Junction>> junctions;
    map<shared_ptr<Junction>, vector<shared_ptr<Line>>> lines;
    map<string, int> stopTimeTable = {{"bus", 1}, {"tram", 2}, {"sprinter", 3}, {"rail", 5}};
    map<string, int> transitTimeTable = {{"intercity",15}, {"central",10}, {"stad", 5}};

};


#endif //NEVERLANDTRANSPORTNETWORK_TRANSPORTNET_H
