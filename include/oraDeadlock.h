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

#ifndef ORADEADLOCK_H
#define ORADEADLOCK_H

#include <string>
#include <map>
#include <vector>

// Vector blows up below if I just use "class" here. Sigh.
#include "oraBlockerWaiter.h"

using std::string;
using std::map;
using std::vector;

class oraTraceFile;

class oraDeadlock
{
    public:
        oraDeadlock(oraTraceFile *tf);
        virtual ~oraDeadlock();
        bool extractDeadlock();
        unsigned lineNumber() { return mLineNumber; }
        void setDateTime(const string date, const string time);
        string date() { return mDate; }
        string time() { return mTime; }
        string dateTime() { return "on " + mDate + " at " + mTime; }
        friend ostream& operator<<(ostream &out, const oraDeadlock &dl);
        vector<string> *signatures();
        vector<string> *waitStack();
        //map<unsigned, oraBlockerWaiter> *blockers();
        //map<unsigned, oraBlockerWaiter> *waiters();
        oraBlockerWaiter *blockerByIndex(const unsigned index);
        oraBlockerWaiter *waiterByIndex(const unsigned index);
        oraBlockerWaiter *blockerBySession(const unsigned session);
        oraBlockerWaiter *waiterBySession(const unsigned session);
        unsigned rows() { return mBlockers.size(); }
        bool txxx();    // Application error? Self Deadlock?
        bool txxs();    // Bitmap Index? ITL? PK/UK inconsistency?
        bool ul();      // User defined lock;
        bool tm();      // Missing FK index?
        void setDeadlockWait(const string reason) { mDeadlockWait = reason; }
        string deadlockWait() { return mDeadlockWait; }
        string SQL() { return mAbortedSQL; }

    private:
        oraTraceFile *mTraceFile;
        unsigned mLineNumber;
        string mDate;
        string mTime;
        map<unsigned, oraBlockerWaiter>mBlockers;
        map<unsigned, oraBlockerWaiter>mWaiters;
        vector<string> mSignatures;
        vector<string>mWaitStack;
        bool sigType(const string what);
        bool extractDeadlockGraph();
        bool extractRowsWaited();
        bool extractCurrentSQL();
        bool extractProcessState();
        bool extractWaitStack();
        string mDeadlockWait;
        string mAbortedSQL;
};

#endif // ORADEADLOCK_H
