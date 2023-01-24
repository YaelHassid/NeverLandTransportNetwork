#include "Line.h"
#include <utility>



Line::Line(const shared_ptr<Junction> &s, const shared_ptr<Junction> &d, int t, const string v) : start(s), destination(d), rideTime(t), vehicle(move(v)) {}

