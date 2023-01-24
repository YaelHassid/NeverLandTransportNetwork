#ifndef NEVERLANDTRANSPORTNETWORK_JUNCTION_H
#define NEVERLANDTRANSPORTNETWORK_JUNCTION_H
#include <string>
using namespace std;
class Junction {
public:
    Junction(const string& type, const string& name);
    string type = "stad";
    string name;

};


#endif //NEVERLANDTRANSPORTNETWORK_JUNCTION_H
