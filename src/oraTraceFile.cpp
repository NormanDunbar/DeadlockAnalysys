#include "oraTraceFile.h"

using std::getline;
using std::ostream;
using std::endl;

//==============================================================================
//                                                                   Constructor
//==============================================================================
oraTraceFile::oraTraceFile(std::string traceFileName)
{
    mIFS = nullptr;
    mLineNumber = 0;
    mPreviousLine = mCurrentLine = mInstanceName = "";
    mRecreatedTraceFile = false;
    mIFS = new ifstream(traceFileName);
    initialise();
}

//==============================================================================
//                                                                    Destructor
//==============================================================================
oraTraceFile::~oraTraceFile()
{
    if (mIFS != nullptr) {
        mIFS->close();
        mIFS = nullptr;
    }
}

//==============================================================================
//                                                                  initialise()
// Extracts various details from the heading lines in the trace file.
//==============================================================================
void oraTraceFile::initialise()
{
    // Read the trace file and extract some "stuff". On exit from here
    // We are sat having just read the first blank line in the trace file.
    mInstanceName = "";
    mOriginalPath = "";
    mSystemName = "";
    mOracleHome = "";
    mServerName = "";

    while (mIFS->good()) {
        readLine();

        if (mCurrentLine.empty()) {
            break;
        }

        if (mCurrentLine.substr(0, 10) == "Trace file") {
            mOriginalPath = mCurrentLine.substr(11);
            continue;
        }

        if (mCurrentLine.substr(0, 11) == "ORACLE_HOME") {
            // This one is different in different systems
            // On some it is "ORACLE_HOME:<some spaces>/path/etc"
            // on others it is "ORACLE_HOME = /path/etc"
            mOracleHome = mCurrentLine.substr(11);
            // Ltrim leading spaces, colons, tabs, equals.
            auto pos = mOracleHome.find_first_not_of(": \t=");
            if (pos != string::npos ) {
                mOracleHome = mOracleHome.substr(pos +1)    ;
            }
            continue;
        }

        if (mCurrentLine.substr(0, 11) == "System name") {
            mSystemName = mCurrentLine.substr(13);
            continue;
        }

        if (mCurrentLine.substr(0, 9) == "Node name") {
            mServerName = mCurrentLine.substr(11);
            continue;
        }

        if (mCurrentLine.substr(0, 13) == "Instance name") {
            mInstanceName = mCurrentLine.substr(15);
            continue;
        }
    }

}

//==============================================================================
//                                                                    readLine()
// Reads the next line from the tracefile. Makes sure that line numbers
// and previous lines are sorted out. Returns the new line read.
//==============================================================================
string oraTraceFile::readLine()
{
    mPreviousLine = mCurrentLine;
    getline(*mIFS, mCurrentLine);

    if (mIFS->good()) {
        mLineNumber++;
        return mCurrentLine;
    };

    // Oops! EOF or error occurred.
    return "";
}

//==============================================================================
//                                                                 findAtStart()
// Looks for some text, case sensitive, at the start of the current
// line from the trace file. If not found, this will keep reading lines until
// it is found or we hit an error or EOF. Returns true if found.
//==============================================================================
bool oraTraceFile::findAtStart(const string lookFor)
{
    auto lookSize = lookFor.length();

    while (mIFS->good()) {
        readLine();
        if (mCurrentLine.substr(0, lookSize) == lookFor ) {
            break;
        }
    }

    return mIFS->good();
}

//==============================================================================
//                                                                findDeadlock()
// Helper to find the start of each deadlock in the tracefile.
//==============================================================================
bool oraTraceFile::findDeadlock()
{
    return findAtStart("DEADLOCK DETECTED");
}

//==============================================================================
//                                                           findDeadlockGraph()
// Helper to find the start of each deadlock graph in the tracefile.
//==============================================================================
bool oraTraceFile::findDeadlockGraph()
{
    return findAtStart("Deadlock graph:");
}

//==============================================================================
//                                                            printInformation()
// Prints out the information in the tracefile header.
//==============================================================================
ostream &oraTraceFile::printInformation(ostream &out)
{
    out << "Original Trace File: " << mOriginalPath << endl
        << "System:              " << mSystemName << endl
        << "Server name:         " << mServerName << endl
        << "Oracle Home:         " << mOracleHome << endl
        << "Instance Name:       " << mInstanceName << endl << endl;

    return out;
}
