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

/*==============================================================================
 * MIT License
 *==============================================================================
 * Copyright (c) 2019 Norman Dunbar
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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
    // Sign on, tell the world who I am!
    cerr << endl << programName << ": v" << programVersion << '\n'
         << "(c)" << programAuthor << '\n' << endl;

    // There must be a single parameter.
    if (argc != 2) {
        usage(ERR_INVALID_PARAMS, "No tracefile name supplied");
    }

    // One parameter received, it's a trace file name, or better be.
    oraTraceFile traceFile(argv[1]);
    if (!traceFile.good()) {
        usage(ERR_INVALID_TRACEFILE, "Cannot open tracefile " + string(argv[1]));
    }

    // Display the trace file details.
    cerr << "Tracefile '" << argv[1] << "' opened.\n" << endl;
    traceFile.printInformation(cerr);

    // Do we have any deadlocks?
    vector<oraDeadlock> deadlocks;
    while (traceFile.findDeadlock()) {
        oraDeadlock temp(traceFile.lineNumber());
        if (temp.extractDeadlockGraph(traceFile)) {
            // We can never hit EOF while extracting the deadlock graph
            // unless, of course, someone has been playing with the trace file.
            deadlocks.push_back(temp);
        } else {
            // Extract failed, somehow. Was it an error or EOF?
            if (!traceFile.eof()) {
                cerr << programName << ": Cannot extract deadlock graph" << endl;
                exit(ERR_TRACEFILE_ERROR);
            }
        }
    }

    cerr << "Extraction complete." << endl
         << "There were " << deadlocks.size()
         << " deadlock(s) found.\n" << endl;

    // List deadlocks in trace file.
    cerr << "DEADLOCKS:\n" << endl;
    for (auto i = deadlocks.begin(); i != deadlocks.end(); i++) {
        cerr << *i << endl;
    }

    deadlocks.clear();
    return 0;
}
