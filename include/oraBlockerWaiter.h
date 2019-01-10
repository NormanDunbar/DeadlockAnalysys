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

#ifndef ORABLOCKERWAITER_H
#define ORABLOCKERWAITER_H

#include <string>
#include <iostream>

using std::string;
using std::ostream;
using std::endl;


// TODO (NDunbar#1#): I could move the setters to the cpp file rather than
// having them inlined as they are here. In case we get too many calls to them
// in the code I'm writing. Bear it in mind - function call overhead or
// inline code overhead? Decisions, decisions!

class oraBlockerWaiter
{
    public:
        oraBlockerWaiter(bool isWaiter = false);
        virtual ~oraBlockerWaiter();

        string resourceName() { return mResourceName; }
        void setResourceName(string val) { mResourceName = val; }

        unsigned session() { return mSession; }
        void setSession(unsigned val) { mSession = val; }

        unsigned process() { return mProcess; }
        void setProcess(unsigned val) { mProcess = val; }

        string holds() { return mHolds; }
        void setHolds(string val);

        string waits() { return mWaits; }
        void setWaits(string val);

        string rowidWait() { return mRowidWait; }
        void setRowidWait(string val) { mRowidWait = val; }

        unsigned objectId() { return mObjectId; }
        void setObjectId(unsigned val) { mObjectId = val; }

        unsigned file() { return mFile; };
        void setFile(unsigned val) { mFile = val; };

        unsigned block() { return mBlock; };
        void setBlock(unsigned val) { mBlock = val; };

        unsigned slot() { return mSlot; };
        void setSlot(unsigned val) { mSlot = val; };

        unsigned otherSession() { return mOtherSession; };
        void setOtherSession(unsigned val) { mOtherSession = val; };

        friend ostream& operator<<(ostream &out, const oraBlockerWaiter &bw);

    protected:

    private:
        bool mIsWaiter;
        string mResourceName;
        unsigned mSession;
        unsigned mProcess;
        string mHolds;
        string mWaits;
        string mRowidWait;
        unsigned mFile;
        unsigned mBlock;
        unsigned mSlot;
        unsigned mObjectId;
        unsigned mOtherSession;
};

#endif // ORABLOCKERWAITER_H
