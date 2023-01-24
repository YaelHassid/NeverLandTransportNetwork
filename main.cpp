#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma ide diagnostic ignored "cert-err60-cpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include "TransportNet.h"
using namespace std;
vector<string> split (const string& s, const string& delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<string> res;

    while ((pos_end = s.find (delimiter, pos_start)) != string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}


void editConfig(const string& filePath, TransportNet& localNet) {
    string line;
    vector<string> splitted;
    ifstream  configuration;
    configuration.open(filePath); //enter the entire path
    while (!configuration.eof()) {
        getline(configuration, line);
        splitted = split(line, " ");
        if (splitted[0] == "bus" || splitted[0] == "tram" || splitted[0] == "sprinter" || splitted[0] == "rail") {
            localNet.stopTimeTable.find(splitted[0])->second = stoi(splitted[1]);
        } else if (splitted[0] == "intercity" || splitted[0] == "central" || splitted[0] == "stad") {
            localNet.transitTimeTable.find(splitted[0])->second = stoi(splitted[1]);
        }
    }
}

void createLine(const string& filePath, TransportNet& localNet) {
    string line, vehicleType, juncType, juncName;
    string cleanFileName = filePath;
    vector<string> splitted;
    vector<string> splitted2;
    ifstream  file;
    file.open(filePath); //enter the entire path
    if (!file) cerr << "ERROR opening the specified file." << endl;
    splitted2 = split(filePath, "\\");
    cleanFileName = splitted2[splitted2.size()-1];
    if (cleanFileName.length() >= 4 && cleanFileName.substr(0,3) == "bus") vehicleType = "bus";
    else if (cleanFileName.length() >= 5 && (cleanFileName.substr(0,4) == "tram" || cleanFileName.substr(0,4) == "rail")) vehicleType = cleanFileName.substr(0,4);
    else if (cleanFileName.length() >= 9 && cleanFileName.substr(0,8) == "sprinter") vehicleType = "sprinter";
//    else throw invalid_argument("the file name must contain one of the vehicles types");
    else cerr << "ERROR opening the specified file." << endl;
    while (!file.eof()) {
        getline(file, line);
        splitted = split(line, "\t");
        //creating the start junction:
        juncType = splitted[0].substr(0,2);
        juncName = splitted[0];
        if (juncType == "IC" || juncType == "CS") juncName = juncName.substr(2, juncName.length());
        const shared_ptr<Junction> start = localNet.addJunction(juncType, juncName);
        // creating the destination junction:
        juncType = splitted[1].substr(0,2);
        juncName = splitted[1];
        if (juncType == "IC" || juncType == "CS") juncName = juncName.substr(2, juncName.length());
        const shared_ptr<Junction> destination = localNet.addJunction(juncType, juncName);
        //creating the route:
        localNet.addLine(start, destination, stoi(splitted[2]), vehicleType);
    }
}

int main( int argc, char** argv) {
    string command, filePath, source, nameCopy, target;
    string outputFileName =  "output.dat";
    TransportNet neverLandTN{};
    try {
        if ((strcmp(argv[1], "-i") != 0 )&& argc > 2)  throw invalid_argument("flag -i is missing");
    } catch (const invalid_argument& e) {
        cerr << "flag -i is missing" << endl;
        return 0;
    }
    try {
        int i = 1;
        if (strcmp(argv[i], "-i") == 0) i++;
        while (argv[i]) {
            if (strcmp(argv[i], "[-c]") == 0) {
                i++;
                editConfig(argv[i], neverLandTN);
            } else if (strcmp(argv[i], "[-o]") == 0){
                i++;
                outputFileName = argv[i];
                break;
            } else {
                createLine(argv[i], neverLandTN);
            }
            i++;
        }
    } catch (const invalid_argument& e) {
        cerr << "the file name must contain one of the vehicles types" << endl;
        return 0;
    }

    while (cin) {
        cin >> command;
        if (command == "load" ) {
            cin >> filePath;
            createLine(filePath, neverLandTN);
            for(const shared_ptr<Junction>& j : neverLandTN.junctions) {
                if (neverLandTN.lines.find(j) != neverLandTN.lines.end()) {
                    for(const shared_ptr<Line>& l : neverLandTN.lines.find(j)->second) {
                        cout << j->name << " " << j->type << " to " << l->destination->name << " time: " << l->rideTime << " with a " << l->vehicle << endl;

                    }
                }
            }

        } else if (command == "outbound") {
            cin >> source;
            shared_ptr<Junction> sourceJunction = nullptr;
            for (const shared_ptr<Junction>& j : neverLandTN.junctions) {
                if (j->name == source) {
                    neverLandTN.bfs(j);
                    break;
                }
            }
            if (sourceJunction == nullptr) {
                cerr << source << " does not exist in the current network." << endl;
            }
        } else if (command == "inbound") {
            cin >> target;
            neverLandTN.bfsReverse(target);
        } else if (command == "uniExpress") {
            cin >> source;
            cin >> target;
            neverLandTN.dijkstraBySameVehicle(source, target, true);
        } else if (command == "multiExpress") {
            cin >> source;
            cin >> target;
            neverLandTN.dijkstraByMultiple(source, target);
        } else if (command == "print") {
            ofstream outputFile;
            outputFile.open (outputFileName);
            for(const shared_ptr<Junction>& j : neverLandTN.junctions) {
                if (neverLandTN.lines.find(j) != neverLandTN.lines.end()) {
                    for(const shared_ptr<Line>& l : neverLandTN.lines.find(j)->second) {
                        outputFile << j->type  << " " << j->name<< " to " << l->destination->name << " time: " << l->rideTime << " with a " << l->vehicle << endl;
                    }
                }
            }
            outputFile.close();
        } else if (command == "EXIT") {
            return 0;
        }
    }
    return 0;
}


#pragma clang diagnostic pop