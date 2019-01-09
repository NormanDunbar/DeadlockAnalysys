/*==============================================================================
 * DeadlockAnalysis
 *==============================================================================
 * A utility to analyse the deadlock traces in an Oracle trace file, and
 * attempt to extract a reason, and as much helpful information as is
 * possible to attempt to resolve the situation.
 *
 * The following Oracle documents have been used to build this utility:
 *
 * 1507093.1 How to identify deadlock types.
 * 1559695.1 How to diagnose different deadlock types.
 * 1552173.1 TX X X Self Deadlock, single row. (Autonomous transaction.)
 * 1552123.1 TX X X Self Deadlock, single row. (No Autonomous transaction.)
 * 1552120.1 TX X X Deadlock with more than one row. (Application code.)
 * 1552169.1 TM SX SSX SX SSX Deadlock. (Missing FK indexes.)
 * 1552175.1 TX X S Deadlock (Bitmap index contention.)
 * 1552191.1 TX X S Deadlock (ITL/PK/Unique Key contention.)
 *------------------------------------------------------------------------------
 * USAGE:
 *
 * DeadlockAnalysis <tracefile_name>
 *------------------------------------------------------------------------------
 * Output is HTML format, and is written to stdout.
 * Errors etc are written to stderr.
 *------------------------------------------------------------------------------
 * Copyright (c) Norman Dunbar January 2019 onwards.
 * Licence: MIT licence. Permission given to use and abuse at your discretion.
 *                       Enjoy!
 *==============================================================================
 */

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector>

using std::string;
using std::cerr;
using std::endl;
using std::vector;


#include "oraTraceFile.h"
#include "oraDeadlock.h"




// Globals. (Yes, I know they are frowned upon - I don't actually care, ok?)
string programName = "DeadlockAnalysis";
string programVersion = "0.1.0";
string programAuthor = "Norman Dunbar (norman@dunbar-it.co.uk)";

#define ERR_INVALID_PARAMS    1
#define ERR_INVALID_TRACEFILE 2
#define ERR_TRACEFILE_ERROR   3

//==============================================================================
//                                                                       USAGE()
//==============================================================================
void usage(int errorCode, string errorText) {
    cerr << endl
         << programName << ": ERROR: " << errorText << endl << endl
         << "USAGE:" << endl
         << "\t" << programName << " tracefile_name >report_name" << endl
         << endl;
    std::exit(errorCode);
}



//==============================================================================
//                                                                        MAIN()
//==============================================================================
int main(int argc, char *argv[])
{
    cerr << endl << programName << ": v" << programVersion << endl
         << programAuthor << endl << endl;
    if (argc != 2) {
        usage(ERR_INVALID_PARAMS, "No tracefile name supplied");
    }

    oraTraceFile traceFile(argv[1]);
    if (!traceFile.good()) {
        usage(ERR_INVALID_TRACEFILE, "Cannot open tracefile " + string(argv[1]));
    }

    cerr << "Tracefile '" << argv[1] << "' opened." << endl << endl;
    traceFile.printInformation(cerr);


    // Do we have any deadlock?
    vector<oraDeadlock> deadlocks;
    while (traceFile.findDeadlock()) {
        oraDeadlock temp(traceFile.lineNumber());
        if (temp.extractDeadlockGraph(traceFile)) {
            deadlocks.push_back(temp);
        } else {
            cerr << programName << ": Cannot extract deadlock graph" << endl;
            exit(ERR_TRACEFILE_ERROR);
        }


    }

    // List deadlocks in trace file.
    cerr << "DEADLOCKS" << endl;
    for (auto i = deadlocks.begin(); i != deadlocks.end(); i++) {
        cerr << "At line: "
             << i->lineNumber() << ". Recorded "
             << i->dateTime()
             << endl;
    }

    deadlocks.clear();
    return 0;
}
