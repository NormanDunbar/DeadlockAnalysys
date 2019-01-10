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

#include "oraBlockerWaiter.h"

string leftTrim(string &s, const string what)
{
    auto pos = s.find_first_not_of(what);
    if (pos != string::npos) {
        return s.substr(pos);
    }

    return s;
}


//==============================================================================
//                                                                   Constructor
//==============================================================================
oraBlockerWaiter::oraBlockerWaiter(bool isWaiter) :
    mIsWaiter(isWaiter)
{
    mSession = 0;
    mProcess = 0;
    mFile = 0;
    mBlock = 0;
    mSlot = 0;
    mObjectId = 0;
}

//==============================================================================
//                                                                    Destructor
//==============================================================================
oraBlockerWaiter::~oraBlockerWaiter()
{
    //dtor
}

//==============================================================================
//                                                                    setHolds()
//==============================================================================
void oraBlockerWaiter::setHolds(string val)
{
    mHolds = leftTrim(val, " \t");
}

//==============================================================================
//                                                                    setWaits()
//==============================================================================
void oraBlockerWaiter::setWaits(string val)
{
    mWaits = leftTrim(val, " \t");
}

//==============================================================================
//                                                                   Operator <<
//------------------------------------------------------------------------------
// Used to dump out an oraBlockerWaiter to a stream, for debugging.
//==============================================================================
ostream& operator<<(ostream &out, const oraBlockerWaiter &bw) {

    // All waiters and blockers have these ...
    out << "\tResource Name:   " << bw.mResourceName << endl
        << "\tProcess:         " << bw.mProcess << endl
        << "\tSession:         " << bw.mSession << endl
        << "\tHolding:         " << bw.mHolds << endl;

    if (bw.mIsWaiter) {
        // But only waiters have these...
        out << "\tWaiting:          " << bw.mWaits << endl
            << "\tBlocking Session: " << bw.mOtherSession << endl
            << "\tRowid Waiting:    " << bw.mRowidWait << endl
            << "\tFile Waiting:     " << bw.mFile << endl
            << "\tBlock Waiting:    " << bw.mBlock << endl
            << "\tSlot Waiting:     " << bw.mSlot << endl
            << "\tObjectID:         " << bw.mObjectId << endl;
    } else {
        // And this is for blockers only.
        out << "\tWaiting Session: " << bw.mOtherSession << endl;
    }

    out << endl;

    return out;
}
