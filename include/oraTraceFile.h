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

#ifndef ORATRACEFILE_H
#define ORATRACEFILE_H

#include <string>
#include <fstream>
#include <iostream>

using std::string;
using std::ifstream;
using std::ostream;

class oraTraceFile
{
    public:
        oraTraceFile(string traceFileName);
        virtual ~oraTraceFile();

    protected:

    private:
        ifstream *mIFS;
        unsigned mLineNumber;
        string mPreviousLine;
        string mCurrentLine;
        bool mRecreatedTraceFile;

        // These are extracted from the trace file.
        string mInstanceName;
        string mOriginalPath;
        string mSystemName;
        string mOracleHome;
        string mServerName;

        void initialise();

    public:
        string currentLine() { return mCurrentLine; };
        string previousLine() { return mPreviousLine; };
        string instanceName() { return mInstanceName; };
        unsigned lineNumber() { return mLineNumber; };
        string traceFile() { return mOriginalPath; };
        string oracleHome() { return mOracleHome; };
        string systemName() { return mSystemName; };
        string serverName() { return mServerName; };
        bool good() { return mIFS->good(); };
        bool eof() { return mIFS->eof(); };
        string readLine();
        bool findAtStart(const string lookFor);
        bool findDeadlock();
        bool findDeadlockGraph();
        ostream &printInformation(ostream &out);

};

#endif // ORATRACEFILE_H
