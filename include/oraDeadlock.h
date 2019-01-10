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

// Vector blows up below if I just use "class" here. Sigh.
#include "oraBlockerWaiter.h"

using std::string;
using std::map;

class oraTraceFile;

class oraDeadlock
{
    public:
        oraDeadlock(unsigned lineNumber);
        virtual ~oraDeadlock();

    protected:

    private:
        unsigned mLineNumber;
        string mDate;
        string mTime;
        map<unsigned, oraBlockerWaiter>mBlockers;
        map<unsigned, oraBlockerWaiter>mWaiters;


    public:
        unsigned lineNumber() { return mLineNumber; };
        void setDateTime(string date, string time);
        string date() { return mDate; };
        string time() { return mTime; };
        string dateTime() { return "on " + mDate + " at " + mTime; };
        bool extractDeadlockGraph(oraTraceFile &traceFile);
        friend ostream& operator<<(ostream &out, const oraDeadlock &dl);

};

#endif // ORADEADLOCK_H
