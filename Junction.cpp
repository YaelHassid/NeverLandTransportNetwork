#include "Junction.h"

Junction::Junction(const string& type, const string& name) {
    this->name = name;
    if (type == "IC") {
        this->type = "intercity";
    }
    else if (type == "CS") {
        this->type = "central";
    }
}
