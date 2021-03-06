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

#include "oraTraceFile.h"


//==============================================================================
//                                                                   Constructor
//==============================================================================
oraTraceFile::oraTraceFile(const string traceFileName):
    mTraceName(traceFileName)
{
    mIFS = nullptr;
    mLineNumber = 0;
    mPreviousLine.reserve(120);
    mCurrentLine.reserve(120);
    mInstanceName.reserve(20);
    mOriginalPath.reserve(200);
    mSystemName.reserve(20);
    mOracleHome.reserve(120);
    mServerName.reserve(20);
    mDeadlocks.reserve(10);


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
//------------------------------------------------------------------------------
// Extracts various details from the heading lines in the trace file.
//==============================================================================
void oraTraceFile::initialise()
{
    // Read the trace file and extract some "stuff". On exit from here
    // We are sat having just read the first blank line in the trace file.

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
//                                                            findAllDeadlocks()
//------------------------------------------------------------------------------
// Finds and extracts all the deadlocks in the trace file and returns the
// number found to the caller.
//==============================================================================
unsigned oraTraceFile::findAllDeadlocks()
{
    unsigned deadlockCount = 0;
    while (mIFS->good()) {
        // Look for another deadlock.
        if (findDeadlock()) {
            cerr << "\tFound a deadlock at line " << mLineNumber << endl;
            deadlockCount++;

            // Create a new deadlock and get it to extract its own details.
            oraDeadlock temp(this);
            temp.extractDeadlock();
            mDeadlocks.push_back(temp);

            // Debug: Dumps out each deadlock at the end. Useful!
            //cerr << "Deadlock: " << deadlockCount << '\n'
            //     << temp << '\n' << std::endl;
        }
    }

    return deadlockCount;
}


//==============================================================================
//                                                                    deadLock()
//------------------------------------------------------------------------------
// Returns a pointer to a single deadlock.
//==============================================================================
oraDeadlock *oraTraceFile::deadLock(const unsigned index)
{
    if (index > mDeadlocks.size() - 1) {
        // Oops! Out of range.
        return nullptr;
    }

    return &(mDeadlocks.at(index));
}

//==============================================================================
//                                                                    readLine()
//------------------------------------------------------------------------------
// Reads the next line from the tracefile. Makes sure that line numbers
// and previous lines are sorted out. Returns the new line read.
//==============================================================================
string oraTraceFile::readLine()
{
    mPreviousLine = mCurrentLine;
    getline(*mIFS, mCurrentLine);

    if (mIFS->good()) {
        mLineNumber++;
        //std::cerr << mLineNumber << ": [" << mCurrentLine << ']' << endl;
        return mCurrentLine;
    };

    // Oops! EOF or error occurred.
    return "";
}

//==============================================================================
//                                                                 findAtStart()
//------------------------------------------------------------------------------
// Looks for some text, case sensitive, at the start of the current
// line from the trace file. If not found, this will keep reading lines until
// it is found or we hit an error or EOF. Returns true if found.
// Returns false if not found, but the end of a deadlock dump is found first, if
// stopAtEndOfDeadlock is true.
//==============================================================================
bool oraTraceFile::findAtStart(const string lookFor, const bool stopAtEndOfDeadlock)
{
    auto lookSize = lookFor.length();

    while (mIFS->good()) {
        readLine();
        if (mCurrentLine.substr(0, lookSize) == lookFor ) {
            return true;
        }

        if (stopAtEndOfDeadlock) {
            if (mCurrentLine == "END OF PROCESS STATE") {
                return false;
            }
        }
    }

    // Return error or EOF.
    return mIFS->good();
}

//==============================================================================
//                                                               findNearStart()
//------------------------------------------------------------------------------
// Looks for some text, case sensitive, at the start of the current
// line from the trace file but with may be prefixed by whitespace.
// If not found, this will keep reading lines until
// it is found or we hit an error or EOF. Returns true if found.
// Returns false if not found, but the end of a deadlock dump is found first if
// stopAtEndOfDeadlock is true.
//==============================================================================
bool oraTraceFile::findNearStart(const string lookFor, const bool stopAtEndOfDeadlock)
{
    auto lookSize = lookFor.length();

    while (mIFS->good()) {
        readLine();
        if (trimmedLine().substr(0, lookSize) == lookFor ) {
            return true;
        }

        if (stopAtEndOfDeadlock) {
            if (mCurrentLine == "END OF PROCESS STATE") {
                return false;
            }
        }
    }

    // Return error or EOF.
    return mIFS->good();
}

//==============================================================================
//                                                                findDeadlock()
//------------------------------------------------------------------------------
// Helper to find the start of each deadlock in the tracefile. Sets  the flag to
// false as we could be still in the middle of a deadlock dump. We need to
// ignore the end of this deadlock, to find the next one.
//==============================================================================
bool oraTraceFile::findDeadlock()
{
    return findAtStart("DEADLOCK DETECTED", false);
}

//==============================================================================
//                                                           findDeadlockGraph()
//------------------------------------------------------------------------------
// Helper to find the start of each deadlock graph in the tracefile.
//==============================================================================
bool oraTraceFile::findDeadlockGraph()
{
    return findAtStart("Deadlock graph:");
}

//==============================================================================
//                                                                 trimmedLine()
//------------------------------------------------------------------------------
// Returns the current line from the trace file, without leading whitespace.
//==============================================================================
string oraTraceFile::trimmedLine()
{
    return mCurrentLine.substr(mCurrentLine.find_first_not_of(" \t"));
}

//==============================================================================
//                                                                   Operator <<
//------------------------------------------------------------------------------
// Used to dump out an oraTraceFile to a stream, for debugging/reporting.
//==============================================================================
ostream& operator<<(ostream &out, const oraTraceFile &tf)
{
    out << "Trace File:          " << tf.mTraceName << '\n'
        << "Original Trace File: " << tf.mOriginalPath << '\n'
        << "System:              " << tf.mSystemName << '\n'
        << "Server name:         " << tf.mServerName << '\n'
        << "Oracle Home:         " << tf.mOracleHome << '\n'
        << "Instance Name:       " << tf.mInstanceName << '\n' << endl;

    return out;
}
