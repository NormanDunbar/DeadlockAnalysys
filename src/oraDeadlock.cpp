/*
 * MIT License
 *
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
 */

#include "oraDeadlock.h"
#include "oraTraceFile.h"

using std::stoi;
using std::cerr;
using std::endl;
using std::pair;
using std::find;


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
    // No pointers involved, so this is good.
    mBlockers.clear();
    mWaiters.clear();
}

//==============================================================================
//                                                                 setDateTime()
//------------------------------------------------------------------------------
// Sets the deadlock's data and time as extracted from the tracefile.
//==============================================================================
void oraDeadlock::setDateTime(string date, string time)
{
    mDate = date;
    mTime = time;
}

//==============================================================================
//                                                        extractDeadlockGraph()
//------------------------------------------------------------------------------
// Extracts relevant information from the tracefile for each deadlock
// found within. An example graph would resemble the following:
//
// Deadlock graph:
//                        ---------Blocker(s)--------  ---------Waiter(s)---------
// Resource Name          process session holds waits  process session holds waits
// TX-0018001f-0025006a       985     272     X            821    1019           S
// TX-0004000e-004a8a86       821    1019     X            779    2156           S
// TX-00360007-001b448f       779    2156     X            985     272           S
//
// session 272: DID 0001-03D9-0000001A	session 1019: DID 0001-0335-00000004
// session 1019: DID 0001-0335-00000004	session 2156: DID 0001-030B-00000004
// session 2156: DID 0001-030B-00000004	session 272: DID 0001-03D9-0000001A
//
// Rows waited on:
//   Session 272: obj - rowid = 004C5C56 - AATFxWAQAAOgakFAAA
//   (dictionary objn - 5004374, file - 1024, block - 243378437, slot - 0)
//   Session 1019: obj - rowid = 004C5C56 - AATFxWAQAAOgdrgAAA
//   (dictionary objn - 5004374, file - 1024, block - 243391200, slot - 0)
//   Session 2156: obj - rowid = 004C5C56 - AATFxWAQAAOgdi7AAA
//   (dictionary objn - 5004374, file - 1024, block - 243390651, slot - 0)
//
// ----- Information for the OTHER waiting sessions -----
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

/*
                       ---------Blocker(s)--------  ---------Waiter(s)---------
Resource Name          process session holds waits  process session holds waits
TX-0018001f-0025006a       985     272     X            821    1019           S
TX-0004000e-004a8a86       821    1019     X            779    2156           S
TX-00360007-001b448f       779    2156     X            985     272           S
*/

    // Find the resources in the deadlock.
    if (!traceFile.findAtStart("Resource Name")) {
        return false;
    }

    oraBlockerWaiter tempBlocker(false), tempWaiter(true);

    deadlockTime = traceFile.readLine();
    while (traceFile.good()) {
        // The resources end at a one-space line.
        if (deadlockTime == " ") {
            break;
        }

        //Extract the blocking session's details.
        auto pos = deadlockTime.find(" ");
        tempBlocker.setResourceName(deadlockTime.substr(0, pos));
        tempBlocker.setProcess(stoi(deadlockTime.substr(23, 7)));
        tempBlocker.setSession(stoi(deadlockTime.substr(31, 7)));
        tempBlocker.setHolds(deadlockTime.substr(39, 5));
        tempBlocker.setWaits(deadlockTime.substr(45, 5));

        //Extract the waiting session's details.
        tempWaiter.setResourceName(tempBlocker.resourceName());
        tempWaiter.setProcess(stoi(deadlockTime.substr(52, 7)));
        tempWaiter.setSession(stoi(deadlockTime.substr(60, 7)));
        tempWaiter.setHolds(deadlockTime.substr(68, 5));
        tempWaiter.setWaits(deadlockTime.substr(74, 5));

        // Set the corresponding other session.
        tempBlocker.setOtherSession(tempWaiter.session());
        tempWaiter.setOtherSession(tempBlocker.session());

        // Save the blocker's details.
        auto ok = mBlockers.insert(pair<unsigned, oraBlockerWaiter>(tempBlocker.session(), tempBlocker));
        if (!ok.second) {
            return false;
        }

        // Save the waiter's details.
        ok = mWaiters.insert(pair<unsigned, oraBlockerWaiter>(tempWaiter.session(), tempWaiter));
        if (!ok.second) {
            return false;
        }

        // Average White Band time ... let's go round again!
        deadlockTime = traceFile.readLine();
    }

    // Skip over the session stuff, not interesting at all.
    if (!traceFile.findAtStart("Rows waited on:")) {
        return false;
    }

    // Extract the objects waited on. I'm using a map<> as I can't guarantee
    // that the order they appear here will always match the creation order.

/*
Rows waited on:
  Session 272: obj - rowid = 004C5C56 - AATFxWAQAAOgakFAAA
  (dictionary objn - 5004374, file - 1024, block - 243378437, slot - 0)
  Session 1019: obj - rowid = 004C5C56 - AATFxWAQAAOgdrgAAA
  (dictionary objn - 5004374, file - 1024, block - 243391200, slot - 0)
  Session 2156: obj - rowid = 004C5C56 - AATFxWAQAAOgdi7AAA
  (dictionary objn - 5004374, file - 1024, block - 243390651, slot - 0)
*/

    while (traceFile.good()) {
        //  Session 272: obj - rowid = 004C5C56 - AATFxWAQAAOgakFAAA
        deadlockTime = traceFile.readLine();

        // The Rows waited on end at a one-space line.
        if (deadlockTime == " ") {
            break;
        }

        auto pos = deadlockTime.find(":");
        unsigned tempNumber = stoi(deadlockTime.substr(9, pos -1));

        // Find the waiter session. If found, waiterPair->second is the Waiter.
        auto waiterPair = mWaiters.find(tempNumber);
        if (waiterPair == mWaiters.end()) {
            // Not found, oops!
            cerr << "Cannot find waiting session: " << tempNumber << endl;
            return false;
        }

        // Rowid waited on.
        string tempString = deadlockTime.substr(deadlockTime.length() -18, 18);
        waiterPair->second.setRowidWait(tempString);

        //  (dictionary objn - 5004374, file - 1024, block - 243378437, slot - 0)
        deadlockTime = traceFile.readLine();

        pos = deadlockTime.find("objn - ");
        tempNumber = stoi(deadlockTime.substr(pos +7));
        waiterPair->second.setObjectId(tempNumber);

        pos = deadlockTime.find("file - ");
        tempNumber = stoi(deadlockTime.substr(pos +7));
        waiterPair->second.setFile(tempNumber);

        pos = deadlockTime.find("block - ");
        tempNumber = stoi(deadlockTime.substr(pos +8));
        waiterPair->second.setBlock(tempNumber);

        pos = deadlockTime.find("slot - ");
        tempNumber = stoi(deadlockTime.substr(pos +7));
        waiterPair->second.setSlot(tempNumber);
    }

    return traceFile.good();
}


//==============================================================================
//                                                                   Operator <<
//------------------------------------------------------------------------------
// Used to dump out an oraDeadlock to a stream, for debugging.
//==============================================================================
ostream& operator<<(ostream &out, const oraDeadlock &dl) {
    out << "Line Number: " << dl.mLineNumber << endl
        << "Date:        " << dl.mDate << endl
        << "Time:        " << dl.mTime << endl
        << "Blockers:    " << dl.mBlockers.size() << endl;

    // List the blockers.
    unsigned x = 0;
    for (auto i = dl.mBlockers.begin(); i != dl.mBlockers.end(); i++) {
        out << "\tBlocker: " << x << endl
            << i->second << endl;
        x++;
    }

    // List the waiters.
    out << "Waiters:     " << dl.mWaiters.size() << endl;
    x = 0;
    for (auto i = dl.mWaiters.begin(); i != dl.mWaiters.end(); i++) {
        out << "\tWaiter: " << x << endl
            << i->second << endl;
        x++;
    }


    return out;
}
