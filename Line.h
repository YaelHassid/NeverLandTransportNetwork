#ifndef NEVERLANDTRANSPORTNETWORK_LINE_H
#define NEVERLANDTRANSPORTNETWORK_LINE_H
#include <string>
#include "Junction.h"
#include <memory>
using namespace std;

class Line {
public:
    Line(const shared_ptr<Junction> &s, const shared_ptr<Junction> &d, int t, string v);

    shared_ptr<Junction> start;
    shared_ptr<Junction> destination;
    int rideTime;
    string vehicle;
    bool visited = false;
};


#endif //NEVERLANDTRANSPORTNETWORK_LINE_H
