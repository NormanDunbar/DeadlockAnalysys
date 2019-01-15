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

#include "oraDeadlockReport.h"


//==============================================================================
//                                                                   Constructor
//==============================================================================
oraDeadlockReport::oraDeadlockReport(oraTraceFile *traceFile):
    mTraceFile(traceFile)
{
    string traceName = traceFile->traceName();
    auto pos = traceName.find_last_of('.');

    // Strip off the current extension and replace it with html.
    mReportName = traceName.substr(0, pos) + ".html";
    mOFS = new ofstream(mReportName);

    // Find the current directory for the trace file.
    string directoryName = "";
    pos = traceName.find_last_of("\\/");
    if (pos != string::npos) {
        directoryName = traceName.substr(0, pos + 1);
    }

    mCssName = directoryName + "DeadlockAnalysis.css";
    ifstream *cssFS = new ifstream(mCssName);
    mCssExists = cssFS->good();
    cssFS->close();
}

//==============================================================================
//                                                                    Destructor
//==============================================================================
oraDeadlockReport::~oraDeadlockReport()
{
    mOFS->close();
}


//==============================================================================
//                                                                      report()
//------------------------------------------------------------------------------
// Writes out a deadlock report for the passed tracefile. The output file is in
// the same location as the trace file, and a CSS stylesheet will be created if
// one doesn't already exist. The HTML report file will be silently overwritten
// if it exists.
//==============================================================================
void oraDeadlockReport::report()
{
    // Do we need a CSS File creating?
    if (!mCssExists) {
        createCSSFile();
    }

    reportHeader();
    reportBody();
    reportFooter();
}

//==============================================================================
//                                                               createCSSFile()
//------------------------------------------------------------------------------
// Writes a new CSS file.
//==============================================================================
void oraDeadlockReport::createCSSFile()
{
    ofstream *cssFS = new ofstream(mCssName);

    if (cssFS->good()) {
        *cssFS << "body {\n"
               << "    background: ivory;\n"
               << "    color: maroon;\n"
               << "}\n\n";

        *cssFS << "table, th, td {\n"
               << "    border: 1px solid bisque;\n"
               << "    padding-left: 4px;\n"
               << "    padding-right: 4px;\n"
               << "    padding-top: 2px;\n"
               << "    padding-bottom: 2px;\n"
               << "    vertical-align: top;\n"
               << "}\n\n";

        *cssFS << "table {\n"
               << "    border-collapse: collapse;\n"
               << "    background: beige;\n"
               << "    font-family: \"courier new\",\"lucida console\",mono;\n"
               << "    font-size: smaller;\n"
               << "    /* If \"fixed\" is not used, the widths are ignored. Sigh */\n"
               << "    /* So the <table> is hard coded as <table style=\"width:95%\"> which I don't like. */\n"
               << "    table-layout: fixed;\n"
               << "    /* Margin is half of 100-95% to accommodate a 95% wide table. */\n"
               << "    margin-left: 2.5%;\n"
               << "}\n\n";

        *cssFS << "pre {\n"
               << "    white-space: pre-wrap;\n"
               << "    word-break: keep-all;\n"
               << "    font-family: \"courier new\";\n"
               << "}\n\n";

        *cssFS << "th {\n"
               << "    background: burlywood;\n"
               << "    color: maroon;\n"
               << "}\n\n";

        *cssFS << ".number {\n"
               << "    text-align: right;\n"
               << "}\n\n";

        *cssFS << ".left {\n"
               << "    text-align: left;\n"
               << "}\n\n";

        *cssFS << ".middle {\n"
               << "    text-align: middle;\n"
               << "}\n\n";

        *cssFS << ".right {\n"
               << "    text-align: right;\n"
               << "}\n\n";

        *cssFS << ".th_tiny {\n"
               << "    width: 10%;\n"
               << "}\n\n";

        *cssFS << ".th_small {\n"
               << "    width: 12%;\n"
               << "}\n\n";

        *cssFS << ".th_medium {\n"
               << "    width: 20%;\n"
               << "}\n\n";

        *cssFS << ".th_large {\n"
               << "    width: 40%;\n"
               << "}\n\n";

        *cssFS << "ul {\n"
               << "    font-size: 0.7em;\n"
               << "}\n\n";

        *cssFS << "li {\n"
               << "}\n\n";

        *cssFS << ".th_small {\n"
               << "    width: 12%;\n"
               << "}\n\n";

        *cssFS << "/*\n"
               << " * Analysis Header details. \n"
               << " * Feel free to modify these to suit your style requirements.\n"
               << " */\n"
               << "#TraceFile {\n"
               << "    font-weight: bold;\n"
               << "}\n\n";

        *cssFS << "#SystemName {\n"
               << "}\n\n";

        *cssFS << "#ServerName {\n"
               << "}\n\n";

        *cssFS << "#OracleHome {\n"
               << "}\n\n";

        *cssFS << "#InstanceName {\n"
               << "    font-weight: bold;\n"
               << "}\n\n";

        *cssFS << "#AnalysisResult {\n"
               << "}\n\n";

        *cssFS << "#DeadlockWait {\n"
               << "    font-weight: bold;\n"
               << "}\n\n";

        *cssFS << "#DeadlockSignature {\n"
               << "}\n\n";

        *cssFS << "#DeadlockCause {\n"
               << "    color: red;\n"
               << "    font-weight: bold;\n"
               << "}\n\n";

        *cssFS << "/*\n"
               << " * If the analysis found deadlocks, change the highlighting.\n"
               << " */\n"
               << ".nonZero {\n"
               << "    color: red;\n"
               << "    font-weight: bold;\n"
               << "}\n\n";

        *cssFS << "/*\n"
               << " * Footer Stuff\n"
               << " */\n";

        *cssFS << ".footer {\n"
               << "    text-align: center;\n"
               << "    font-size: x-small;\n"
               << "}\n\n";

        *cssFS << ".url {\n"
               << "    color: blue;\n"
               << "    font-family: \"courier new\";\n"
               << "}\n";

        // Flush.
        *cssFS << endl;
    }
}

//==============================================================================
//                                                                reportHeader()
//------------------------------------------------------------------------------
// Writes an HTML file header.
//==============================================================================
void oraDeadlockReport::reportHeader()
{
    *mOFS << "<html>\n"
             "<head>\n"
             "<title>Deadlock Analysis</title>\n"
             "<link rel=\"stylesheet\" href=\"DeadlockAnalysis.css\">\n"
             "</head>\n"
             "<body>" << endl;
}

//==============================================================================
//                                                                reportFooter()
//------------------------------------------------------------------------------
// Writes an HTML file footer.
//==============================================================================
void oraDeadlockReport::reportFooter()
{
    extern string programName;
    extern string programVersion;
    extern string programAuthor;

    *mOFS << "<p></p>\n<hr>\n"
          << "<p class=\"footer\">"
          << "Created with <strong>" << programName << ' ' << programVersion
          << "</strong><br>Copyright &copy; " << programAuthor << " 2018/19<br>\n"
          << "Released under the <a href=\"https://opensource.org/licenses/MIT\"><span class=\"url\">MIT Licence</span></a><br><br>\n"
          << "Binary releases available from: "
          << "<a href=\"https://github.com/NormanDunbar/DeadlockAnalysys/releases\">"
          << "<span class=\"url\">https://github.com/NormanDunbar/DeadlockAnalysys/releases</span></a><br>\n"
          << "Source code available from: "
          << "<a href=\"https://github.com/NormanDunbar/DeadlockAnalysys\">"
          << "<span class=\"url\">https://github.com/NormanDunbar/DeadlockAnalysys</span></a>\n\n";

    *mOFS << "</body>\n"
             "</html>" << endl;
}

//==============================================================================
//                                                                  reportBody()
//------------------------------------------------------------------------------
// Writes an HTML report for the supplied trace file. This is in a number of
// sections:
//  Details of the trace file itself;
//  A "quick index" with links to each actual deadlock;
// The deadlocks themselves.
//==============================================================================
void oraDeadlockReport::reportBody()
{
    traceFileDetails();
    quickIndex();
    deadlocks();

}

//==============================================================================
//                                                            traceFileDetails()
//------------------------------------------------------------------------------
// Writes the top section of the report with details of the trace file that has
// been analysed.
//==============================================================================
void oraDeadlockReport::traceFileDetails()
{
    heading(1, "Deadlock Analysis");
    heading(2, "Tracefile Summary");

    // Open the table.
    *mOFS << "<table  style=\"width:95%\">\n";

    // Local trace file name.
    *mOFS << "<tr><th class=\"right th_small\">Trace File</th>"
          << "<td id=\"TraceFile\">"
          << mTraceFile->traceName()
          << "</td></tr>\n";

    // Close the table.
    *mOFS << "</table><br>\n\n";

    // Open the table.
    *mOFS << "<table  style=\"width:95%\">\n";

    // Original trace file name on the server.
    *mOFS << "<tr><th class=\"right th_small\">Original Trace File</th>"
          << "<td id=\"TraceFile\">"
          << mTraceFile->originalPath()
          << "</td></tr>\n";

    // System name.
    *mOFS << "<tr><th class=\"right th_small\">System</th>"
          << "<td id=\"SystemName\">"
          << mTraceFile->systemName()
          << "</td></tr>\n";

    // Server Name.
    *mOFS << "<tr><th class=\"right th_small\">Server name</th>"
          << "<td id=\"ServerName\">"
          << mTraceFile->serverName()
          << "</td></tr>\n";

    // Oracle Home.
    *mOFS << "<tr><th class=\"right th_small\">Oracle Home</th>"
          << "<td id=\"OracleHome\">"
          << mTraceFile->oracleHome()
          << "</td></tr>\n";

    // Instance Name.
    *mOFS << "<tr><th class=\"right th_small\">Instance Name</th>"
          << "<td id=\"InstanceName\">"
          << mTraceFile->instanceName()
          << "</td></tr>\n";

    // How many deadlocks were found?
    unsigned deadlockCount = mTraceFile->deadlockCount();
    *mOFS << "<tr><th class=\"right th_small\">Analysis</td>"
          << "<td id=\"AnalysisResult\" class=\""
          << (deadlockCount != 0 ? "nonZero" : "number")
          << "\"> There were " << deadlockCount << " deadlock"
          << (deadlockCount == 1 ? "" : "s")
          << " in the trace file"
          << "</td></tr>\n";

    // Close the table.
    *mOFS << "</table>\n" << endl;
}

//==============================================================================
//                                                                  quickIndex()
//------------------------------------------------------------------------------
// Writes the quick index links, one link to each deadlock found in the trace
// file. Helpful in the event we have many deadlocks due to the system being up
// for a while.
//==============================================================================
void oraDeadlockReport::quickIndex()
{
    heading(2, "Deadlock Details");
    heading(3, "Quick Index");

    *mOFS << "<ul>\n";

    for (unsigned x = 0; x < mTraceFile->deadlockCount(); x++) {
        *mOFS << "<li><a href=\"#deadlock_" << x + 1 << "\">"
              << "Deadlock " << x + 1 << "</a></li>\n";
    }

    *mOFS << "</ul>\n" << endl;
}

//==============================================================================
//                                                                   deadlocks()
//------------------------------------------------------------------------------
// Writes one <div> for each deadlock found in the trace file. The <div> has an
// ID, which is the destination of the links in the quick index.
//==============================================================================
void oraDeadlockReport::deadlocks()
{
    for (unsigned x = 0; x < mTraceFile->deadlockCount(); x++) {
        oraDeadlock *thisDeadlock = mTraceFile->deadLock(x);

        // Open the div.
        *mOFS << "<div id=\"deadlock_" << x + 1 << "\">\n";
        heading(3, "Deadlock " + to_string(x + 1));

        // Deadlock summary first.
        deadlockSummary(thisDeadlock);

        // Deadlock graph.
        deadlockGraph(thisDeadlock);

        // Waiter details.
        deadlockWaiters(thisDeadlock);

        // Close the div.
        *mOFS << "</div>\n" << endl;
    }
}

//==============================================================================
//                                                             deadlockSummary()
//------------------------------------------------------------------------------
// Dumps out a summary of a single deadlock.
//==============================================================================
void oraDeadlockReport::deadlockSummary(oraDeadlock *dl)
{
    heading(4, "Deadlock Summary");
    // Open the table.
    *mOFS << "<table  style=\"width:95%\">\n";

    // Line number.
    *mOFS << "<tr><th class=\"right th_small\">Line Number</th>"
          << "<td class=\"left\">"
          << dl->lineNumber()
          << "</td></tr>\n";

    // Sessions involved in the deadlock.
    *mOFS << "<tr><th class=\"right th_small\">Sessions involved</th>"
          << "<td class=\"left\">"
          << dl->rows()
          << "</td></tr>\n";

    // Deadlock wait reason.
    *mOFS << "<tr><th class=\"right th_small\">Deadlock Wait</th>"
          << "<td id=\"DeadlockWait\">"
          << dl->deadlockWait()
          << "</td></tr>\n";

    // Deadlock signature.
    *mOFS << "<tr><th class=\"right th_small\">Deadlock Signature</th>"
          << "<td id=\"DeadlockSignature\">";

          for (unsigned s = 0; s < dl->signatures()->size(); s++) {
              *mOFS << dl->signatures()->at(s)
                    << "<br>";
          }

    *mOFS << "</td></tr>\n";

    // Probable cause.
    bool gotCause = false;
    *mOFS << "<tr><th class=\"right th_small\">Probable Cause</th>"
          << "<td id=\"DeadlockCause\">";

    if (dl->ul()) {
        gotCause = true;
        *mOFS << "UL: User defined locking - you are, unfortunately, on your own!<br>\n"
              << "    Check for inappropriate use of DBMS_LOCK, perhaps, or LOCK TABLE.<br>\n";
    }

    if (dl->txxs()) {
        gotCause = true;
        *mOFS << "TX-X-S: Insufficient ITL entries (See docId 1552191.1); or,<br>\n"
              << "TX-X-S: Bitmap indexes (See docId 1552175.1); or,<br>\n"
              << "TX-X-S: Manipulation primary/unique key in an inconsistent order (See docId 1552191.1).<br>\n";
    }

    if (dl->tm()) {
        gotCause = true;
        *mOFS << "TM: Unindexed FK constraint columns (See docId 1552169.1).<br>\n";
    }

    if (dl->txxx()) {
        gotCause = true;
        if (dl->rows() == 1) {
            *mOFS << "TX-X-X: Self deadlock - with an autonomous transaction (See docId 1552173); or,<br>\n"
                  << "TX-X-X: Self deadlock - without an autonomous transaction (See docId 1552123).<br>\n";
        } else {
            *mOFS << "TX-X-X: Deadlock caused by application code (See docId 1552120.1).<br>\n";
        }
    }

    if (!gotCause) {
        *mOFS << "Deadlock cause unknown. Please zip and send this trace file - after obfuscating any personal "
              << "data or server names, IP addresses, just in case - to Norm via Github as an issue "
              << "and he'll attempt to find the cause and fix the code to avoid this in future.<br>\n"
              << "The Issues URL is <a href=\"https://github.com/NormanDunbar/DeadlockAnalysys/issues\">"
              << "<span class=\"url\">https://github.com/NormanDunbar/DeadlockAnalysys/issues</span></a>.<br>";
    }
    *mOFS << "</td></tr>\n";

    // Aborted SQL
    *mOFS << "<tr><th class=\"right th_small\">Aborted SQL</th>"
          << "<td><pre>" << dl->SQL() << "</pre></td></tr>\n";

    // Close the table.
    *mOFS << "</table>\n";

}

//==============================================================================
//                                                               deadlockGraph()
//------------------------------------------------------------------------------
// Dumps out details of a single deadlock graph.
//==============================================================================
void oraDeadlockReport::deadlockGraph(oraDeadlock *dl)
{
    heading(4, "Deadlock Graph");

    // Open the table.
    *mOFS << "<table  style=\"width:95%\">\n";

    // Headings for table.
    *mOFS << "<tr><th class=\"th_medium\">&nbsp;</th>"
          << "<th colspan=4 class=\"th_large\">Blockers</th>"
          << "<th colspan=4 class=\"th_large\">Waiters</th></tr>\n";

    *mOFS << "<tr><th>Resource Name</th>";
    for (unsigned x = 0; x < 2; x++) {
        // Two sets of headings here.
        *mOFS << "<th>Process</th>"
              << "<th>Session</th>"
              << "<th>Holding</th>"
              << "<th>Waiting</th>";
    }
    *mOFS << "</tr>\n";

    // Process all the blockers, and whoever is waiting for them.
    for (unsigned x = 0; x < dl->rows(); x++) {
        oraBlockerWaiter *b = dl->blockerByIndex(x);

        if (b) {
            // Get the waiter for this blocker.
            oraBlockerWaiter *w = dl->waiterBySession(b->otherSession());

            // Resource name.
            *mOFS << "<tr><td>" << b->resourceName() << "</td>\n";

            // Blocker details
            *mOFS << "<td class=\"middle\">" << b->process() << "</td>"
                  << "<td class=\"middle\">" << b->session() << "</td>"
                  << "<td class=\"middle\">" << (b->holds().empty() ? "&nbsp;" : b->holds()) << "</td>"
                  << "<td class=\"middle\">" << (b->waits().empty() ? "&nbsp;" : b->waits()) << "</td>";

            // Waiter details
            *mOFS << "<td class=\"middle\">" << w->process() << "</td>"
                  << "<td class=\"middle\">" << w->session() << "</td>"
                  << "<td class=\"middle\">" << (w->holds().empty() ? "&nbsp;" : w->holds()) << "</td>"
                  << "<td class=\"middle\">" << (w->waits().empty() ? "&nbsp;" : w->waits()) << "</td>";

            *mOFS << "</tr>\n";
        }

    }

    // Close the table.
    *mOFS << "</table>\n";

}

//==============================================================================
//                                                             deadlockWaiters()
//------------------------------------------------------------------------------
// Dumps out details of deadlock's waiters.
//==============================================================================
void oraDeadlockReport::deadlockWaiters(oraDeadlock *dl)
{
    heading(4, "Deadlock Waiters");

    // Open the table.
    *mOFS << "<table  style=\"width:95%\">\n";

    // Headings.
    *mOFS << "<tr><th class=\"th_medium\">Resource Name</th>"
          << "<th class=\"th_tiny\">Session</th>"
          << "<th class=\"th_tiny\">Blocker</th>"
          << "<th class=\"th_medium\">Rowid Waited</th>"
          << "<th class=\"th_tiny\">File No.</th>"
          << "<th class=\"th_tiny\">Block No.</th>"
          << "<th class=\"th_tiny\">Slot No.</th>"
          << "<th class=\"th_tiny\">Object Id</th>\n";

    *mOFS << "</tr>\n";

    // Grab the waiters in the order they are in the deadlock graph
    // so, basically, process the blockers and grab their waiting session.
    for (unsigned x = 0; x < dl->rows(); x++) {
        oraBlockerWaiter *b = dl->blockerByIndex(x);

        if (b) {
            // Get the waiter for this blocker.
            oraBlockerWaiter *w = dl->waiterBySession(b->otherSession());

            // Resource name.
            *mOFS << "<tr><td class=\"left\">" << w->resourceName() << "</td>\n";

            // Session.
            *mOFS << "<td class=\"middle\">" << w->session() << "</td>\n";

            // Blocker.
            *mOFS << "<td class=\"middle\">" << w->otherSession() << "</td>\n";

            // Rowid.
            *mOFS << "<td class=\"middle\">" << w->rowidWait() << "</td>\n";

            // File.
            *mOFS << "<td class=\"middle\">" << w->file() << "</td>\n";

            // Block.
            *mOFS << "<td class=\"middle\">" << w->block() << "</td>\n";

            // Slot.
            *mOFS << "<td class=\"middle\">" << w->slot() << "</td>\n";

            // Object Id.
            *mOFS << "<td class=\"middle\">" << w->objectId() << "</td>\n";

            // Close the row.
            *mOFS << "</tr>\n";

            //
        }
    }

    // Close the table.
    *mOFS << "</table>\n";

}

//==============================================================================
//                                                                     heading()
//------------------------------------------------------------------------------
// Writes an HTML heading line of a given heading level.
//==============================================================================
void oraDeadlockReport::heading(const unsigned level, const string heading)
{
    *mOFS << "<h" << level << '>' << heading << "</h" << level << ">\n" << endl;
}

