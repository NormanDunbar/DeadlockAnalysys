#ifndef ORADEADLOCK_H
#define ORADEADLOCK_H

#include <string>

using std::string;

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


    public:
        unsigned lineNumber() { return mLineNumber; };
        void setDateTime(string date, string time);
        string date() { return mDate; };
        string time() { return mTime; };
        string dateTime() { return "on " + mDate + " at " + mTime; };
        bool extractDeadlockGraph(oraTraceFile &traceFile);

};

#endif // ORADEADLOCK_H
