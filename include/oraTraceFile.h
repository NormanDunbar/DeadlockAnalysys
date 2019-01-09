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
        string readLine();
        bool findAtStart(const string lookFor);
        bool findDeadlock();
        bool findDeadlockGraph();
        ostream &printInformation(ostream &out);

};

#endif // ORATRACEFILE_H
