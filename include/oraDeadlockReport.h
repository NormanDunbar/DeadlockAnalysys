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



#ifndef ORADEADLOCKREPORT_H
#define ORADEADLOCKREPORT_H

#include <fstream>
#include <string>

#include "oraTraceFile.h"

using std::ifstream;
using std::ofstream;
using std::string;
using std::to_string;


class oraDeadlockReport
{
    public:
        oraDeadlockReport(oraTraceFile *traceFile);
        bool good() { return mOFS->good(); }
        string reportName() { return mReportName; }
        virtual ~oraDeadlockReport();
        void report();

    protected:

    private:
        oraTraceFile *mTraceFile;
        string mReportName;
        string mCssName;
        ofstream *mOFS;
        void createCSSFile();
        void reportHeader();
        void reportFooter();
        void reportBody();
        void traceFileDetails();
        void quickIndex();
        void deadlocks();
        void deadlockSummary(oraDeadlock *dl);
        void deadlockGraph(oraDeadlock *dl);
        void deadlockWaiters(oraDeadlock *dl);
        void heading(const unsigned level, const string heading);
        bool mCssExists;

};

#endif // ORADEADLOCKREPORT_H
