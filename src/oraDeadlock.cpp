#include "oraDeadlock.h"
#include "oraTraceFile.h"

#include <string>


using std::string;


//==============================================================================
//                                                                   Constructor
//==============================================================================
oraDeadlock::oraDeadlock(unsigned lineNumber):
    mLineNumber{lineNumber}
{
}

//==============================================================================
//                                                                    Destructor
//==============================================================================
oraDeadlock::~oraDeadlock()
{
}

//==============================================================================
//                                                                 setDateTime()
// Sets the deadlock's data and time as extracted from the tracefile.
//==============================================================================
void oraDeadlock::setDateTime(string date, string time)
{
    mDate = date;
    mTime = time;
}

//==============================================================================
//                                                        extractDeadlockGraph()
// Extracts relevant information from the tracefile for each deadlock
// found within.
//==============================================================================
bool oraDeadlock::extractDeadlockGraph(oraTraceFile &traceFile)
{
    // Date and time of this deadlock.
    string deadlockTime = traceFile.previousLine();
    mDate = deadlockTime.substr(4, 10);
    mTime = deadlockTime.substr(15, 8);

    // Find the deadlock graph...
    if (!traceFile.findDeadlockGraph()) {
        return false;
    }

    // Find the resources in the deadlock.
    if (!traceFile.findAtStart("Resource Name")) {
        return false;
    }


    return traceFile.good();
}
