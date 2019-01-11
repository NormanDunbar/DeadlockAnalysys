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

//==============================================================================
//                                                                    leftTrim()
//------------------------------------------------------------------------------
// Returns a string, trimmed of any leading characters requested to be removed.
// If the string is all the requested characters, return "".
//==============================================================================
string leftTrim(string &s, const string what)
{
    auto pos = s.find_first_not_of(what);
    if (pos != string::npos) {
        return s.substr(pos);
    }

    return "";
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

    // Preallocate string space.
    mResourceName.reserve(20);
    mHolds.reserve(3);
    mWaits.reserve(3);
    mRowidWait.reserve(20);
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
//------------------------------------------------------------------------------
// Updates the lock held by this blocker/waiter.
//==============================================================================
void oraBlockerWaiter::setHolds(string val)
{
    mHolds = leftTrim(val, " \t");
}

//==============================================================================
//                                                                    setWaits()
//------------------------------------------------------------------------------
// Updates the lock held by this blocker/waiter.
//==============================================================================
void oraBlockerWaiter::setWaits(string val)
{
    mWaits = leftTrim(val, " \t");
}

//==============================================================================
//                                                                   Operator <<
//------------------------------------------------------------------------------
// Used to dump out an oraBlockerWaiter to a stream, for debugging/reporting.
//==============================================================================
ostream& operator<<(ostream &out, const oraBlockerWaiter &bw) {

    // All waiters and blockers have these ...
    out << "\tResource Name:   " << bw.mResourceName << '\n'
        << "\tProcess:         " << bw.mProcess << '\n'
        << "\tSession:         " << bw.mSession << '\n'
        << "\tHolding:         " << (bw.mHolds.empty() ? "None" : bw.mHolds) << '\n'
        << "\tWaiting:         " << (bw.mWaits.empty() ? "None" : bw.mWaits) << '\n';

    if (bw.mIsWaiter) {
        // But only waiters have these...
        out << "\tBlocking Session: " << bw.mOtherSession << '\n'
            << "\tRowid Waiting:    " << bw.mRowidWait << '\n'
            << "\tFile Waiting:     " << bw.mFile << '\n'
            << "\tBlock Waiting:    " << bw.mBlock << '\n'
            << "\tSlot Waiting:     " << bw.mSlot << '\n'
            << "\tObjectID:         " << bw.mObjectId << '\n';
    } else {
        // And this is for blockers only.
        out << "\tWaiting Session: " << bw.mOtherSession << '\n';
    }

    out << endl;

    return out;
}

