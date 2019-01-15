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
oraDeadlock::oraDeadlock(oraTraceFile *tf):
    mTraceFile(tf)
{
    // Get the line number.
    mLineNumber = tf->lineNumber();

    // Preallocate strings.
    mDate.reserve(10);
    mTime.reserve(10);
    mAbortedSQL.reserve(2048);

    // Allocate space for 5 strings.
    mSignatures.reserve(5);
}

//==============================================================================
//                                                                    Destructor
//==============================================================================
oraDeadlock::~oraDeadlock()
{
    // No pointers involved, so this is good.
    mBlockers.clear();
    mWaiters.clear();
    mSignatures.clear();
}

//==============================================================================
//                                                                 setDateTime()
//------------------------------------------------------------------------------
// Sets the deadlock's data and time as extracted from the tracefile.
//==============================================================================
void oraDeadlock::setDateTime(const string date, const string time)
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
bool oraDeadlock::extractDeadlockGraph()
{
    // Extract the Date and time of this deadlock.
    // *** 2018-12-19 15:42:20.941....
    string deadlockTime = mTraceFile->previousLine();
    auto pos = deadlockTime.find(' ');
    auto pos2 = deadlockTime.find(' ', pos + 1);
    mDate = deadlockTime.substr(pos + 1, pos2 - pos -1);
    mTime = deadlockTime.substr(pos2 + 1, 8);

    // Find the deadlock graph...
    if (!mTraceFile->findDeadlockGraph()) {
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
    if (!mTraceFile->findAtStart("Resource Name")) {
        cerr << "Cannot find [Resource Name]" << endl;
        return false;
    }

    // Each resource should have a blocker and waiter.
    oraBlockerWaiter tempBlocker(false), tempWaiter(true);

    // And a signature.
    string signature;

    deadlockTime = mTraceFile->readLine();
    while (mTraceFile->good()) {
        // The resources end at a one-space line.
        if (deadlockTime == " ") {
            break;
        }

        //Extract the blocking session's details.
        signature = deadlockTime.substr(0, 2) + '-';

        auto pos = deadlockTime.find(" ");
        tempBlocker.setResourceName(deadlockTime.substr(0, pos));
        tempBlocker.setProcess(stoi(deadlockTime.substr(23, 7)));
        tempBlocker.setSession(stoi(deadlockTime.substr(31, 7)));
        tempBlocker.setHolds(deadlockTime.substr(39, 5));
        signature += (tempBlocker.holds().empty() ? "" : tempBlocker.holds());
        tempBlocker.setWaits(deadlockTime.substr(45, 5));
        signature += (tempBlocker.waits().empty() ? "" : tempBlocker.waits());

        //Extract the waiting session's details.
        tempWaiter.setResourceName(tempBlocker.resourceName());
        tempWaiter.setProcess(stoi(deadlockTime.substr(52, 7)));
        tempWaiter.setSession(stoi(deadlockTime.substr(60, 7)));
        tempWaiter.setHolds(deadlockTime.substr(68, 5));
        signature += '-' + (tempWaiter.holds().empty() ? "" : tempWaiter.holds());
        tempWaiter.setWaits(deadlockTime.substr(74, 5));
        signature += (tempWaiter.waits().empty() ? "" : tempWaiter.waits());

        // Set the corresponding other session.
        tempBlocker.setOtherSession(tempWaiter.session());
        tempWaiter.setOtherSession(tempBlocker.session());

        // Save the deadlock signature if the list is currently empty, or
        // if we don't have this signature already. I know it's a palava
        // scanning the vector, but it's only small.
        vector<string>::iterator i;
        for (i = mSignatures.begin(); i != mSignatures.end(); i++) {
            if (*i == signature) {
                // Exists, stop looking.
                break;
            }
        }

        // If we hit the end, add it. Even if the final string was equal
        // we won't be sitting at mSignatures.end() as that is past the end.
        if (i == mSignatures.end()) {
            mSignatures.push_back(signature);
        }

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
        deadlockTime = mTraceFile->readLine();
    }

    // Skip over the session stuff, not interesting at all.
    if (!mTraceFile->findAtStart("Rows waited on:")) {
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

    while (mTraceFile->good()) {
        //  Session 272: obj - rowid = 004C5C56 - AATFxWAQAAOgakFAAA
        deadlockTime = mTraceFile->readLine();

        // The Rows waited on end at a one-space line.
        if (deadlockTime == " ") {
            break;
        }

        auto pos = deadlockTime.find(":");
        unsigned tempNumber = stoi(deadlockTime.substr(9, pos -1));

        // Find the waiter session.
        auto thisWaiter = waiterBySession(tempNumber);

        //auto waiterPair = mWaiters.find(tempNumber);
        if (!thisWaiter) {
            // Not found, oops!
            cerr << "Cannot find waiting session: " << tempNumber << endl;
            return false;
        }

        // Rowid waited on.
        string tempString = deadlockTime.substr(deadlockTime.length() -18, 18);
        thisWaiter->setRowidWait(tempString);

        //  (dictionary objn - 5004374, file - 1024, block - 243378437, slot - 0)
        deadlockTime = mTraceFile->readLine();

        pos = deadlockTime.find("objn - ");
        tempNumber = stoi(deadlockTime.substr(pos +7));
        thisWaiter->setObjectId(tempNumber);

        pos = deadlockTime.find("file - ");
        tempNumber = stoi(deadlockTime.substr(pos +7));
        thisWaiter->setFile(tempNumber);

        pos = deadlockTime.find("block - ");
        tempNumber = stoi(deadlockTime.substr(pos +8));
        thisWaiter->setBlock(tempNumber);

        pos = deadlockTime.find("slot - ");
        tempNumber = stoi(deadlockTime.substr(pos +7));
        thisWaiter->setSlot(tempNumber);
    }

        // Extract the aborted SQL statement.
        if (!mTraceFile->findAtStart("----- Current SQL Statement")) {
            cerr << "Cannot find [----- Current SQL Statement]" << endl;
            return false;
        }

        mTraceFile->readLine();
        while (true) {
            unsigned length = mTraceFile->currentLine().size();
            if (length >= 10) {
                if (mTraceFile->currentLine().substr(0, 10) == "==========") {
                    break;
                }
            }

            mAbortedSQL += mTraceFile->currentLine();
            mTraceFile->readLine();
        }


        // Scan for the reason we are waiting, it's in the process state.
        if (!mTraceFile->findAtStart("PROCESS STATE")) {
            cerr << "Cannot find [PROCESS STATE]" << endl;
            return false;
        }

        // Scan to a line with the "current wait stack" in it.
        if (!mTraceFile->findNearStart("Current Wait Stack:")) {
            cerr << "Cannot find [Current Wait Stack:]" << endl;
            return false;
        }

        // Now we have a look for the reason we deadlocked this
        // session which is on the following line.
        mTraceFile->readLine();
        deadlockTime = mTraceFile->trimmedLine();
        setDeadlockWait(deadlockTime.substr(3));



    return mTraceFile->good();
}

//==============================================================================
//                                                                  signatures()
//------------------------------------------------------------------------------
// Returns a pointer to the list of signatures for this deadlock.
//==============================================================================
vector<string> *oraDeadlock::signatures()
{
    return &mSignatures;
}

//==============================================================================
//                                                                    blockers()
//------------------------------------------------------------------------------
// Returns a pointer to the list of blockers for this deadlock.
//==============================================================================
//map<unsigned, oraBlockerWaiter> *oraDeadlock::blockers()
//{
//    return &mBlockers;
//}

//==============================================================================
//                                                                     waiters()
//------------------------------------------------------------------------------
// Returns a pointer to the list of waiters for this deadlock.
//==============================================================================
//map<unsigned, oraBlockerWaiter> *oraDeadlock::waiters()
//{
//    return &mWaiters;
//}


//==============================================================================
//                                                            blockerBySession()
//------------------------------------------------------------------------------
// Returns a pointer to a blocking session.
//==============================================================================
oraBlockerWaiter *oraDeadlock::blockerBySession(const unsigned session)
{
    auto b = mBlockers.find(session);
    if (b != mBlockers.end()) {
        return &(b->second);
    }

    return nullptr;
}


//==============================================================================
//                                                             waiterBySession()
//------------------------------------------------------------------------------
// Returns a pointer to a waiting session.
//==============================================================================
oraBlockerWaiter *oraDeadlock::waiterBySession(const unsigned session)
{
    auto w = mWaiters.find(session);
    if (w != mWaiters.end()) {
        return &(w->second);
    }

    return nullptr;
}

//==============================================================================
//                                                              blockerByIndex()
//------------------------------------------------------------------------------
// Returns a pointer to a blocking session. This is array type access to a map
// which it seems is not specifically allowed without a key. B*gg*r! Have to be
// sneaky now.
//==============================================================================
oraBlockerWaiter *oraDeadlock::blockerByIndex(const unsigned index)
{
    if (index >= mBlockers.size()) {
        return nullptr;
    }

    unsigned currentIndex = 0;
    for (auto i = mBlockers.begin(); i != mBlockers.end(); i++) {
        if (currentIndex == index) {
            return &(i->second);
        }

        currentIndex++;
    }

    return &(mBlockers[index]);
}


//==============================================================================
//                                                               waiterByIndex()
//------------------------------------------------------------------------------
// Returns a pointer to a waiting session.
//==============================================================================
oraBlockerWaiter *oraDeadlock::waiterByIndex(const unsigned index)
{
    if (index > mWaiters.size()) {
        return nullptr;
    }

    unsigned currentIndex = 0;
    for (auto i = mWaiters.begin(); i != mWaiters.end(); i++) {
        if (currentIndex == index) {
            return &(i->second);
        }

        currentIndex++;
    }

    return &(mWaiters[index]);
}

//==============================================================================
//                                                                     ???????()
//------------------------------------------------------------------------------
// The following boolean functions return true if any of the deadlock signatures
// match that requested.
//==============================================================================
bool oraDeadlock::sigType(const string what) {
    for (auto i = mSignatures.begin(); i != mSignatures.end(); i++) {
        if ((*i).substr(0, what.size()) == what) {
            return true;
        }
    }

    return false;
}

bool oraDeadlock::txxx()    // Application error? Self Deadlock?
{
    return sigType("TX-X-X");
}

bool oraDeadlock::txxs()    // Bitmap Index? ITL? PK/UK inconsistency?
{
    return sigType("TX-X-S");
}

bool oraDeadlock::ul()      // User defined lock;
{
    return sigType("UL");
}

bool oraDeadlock::tm()      // Missing FK index?
{
    return sigType("TM-SX-SSX-SX-SSX");
}




//==============================================================================
//                                                                   Operator <<
//------------------------------------------------------------------------------
// Used to dump out an oraDeadlock to a stream, for debugging/reporting.
//==============================================================================
ostream& operator<<(ostream &out, const oraDeadlock &dl) {
    out << "Line Number:  " << dl.mLineNumber << '\n'
        << "Date:         " << dl.mDate << '\n'
        << "Time:         " << dl.mTime << '\n'
        << "Blockers:     " << dl.mBlockers.size() << "\n\n";

    // List the blockers.
    unsigned x = 0;
    for (auto i = dl.mBlockers.begin(); i != dl.mBlockers.end(); i++) {
        out << "\tBlocker: " << x << '\n'
            << "\t-------\n"
            << i->second << '\n';
        x++;
    }

    // List the waiters.
    out << "Waiters:     " << dl.mWaiters.size() << "\n\n";
    x = 0;
    for (auto i = dl.mWaiters.begin(); i != dl.mWaiters.end(); i++) {
        out << "\tWaiter: " << x << '\n'
            << "\t------\n"
            << i->second << endl;
        x++;
    }

    return out;
}
