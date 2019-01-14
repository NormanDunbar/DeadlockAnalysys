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
               << "    font-size: larger;\n"
               << "}\n\n";

        *cssFS << "th {\n"
               << "    background: burlywood;\n"
               << "    color: maroon;\n"
               << "}\n\n";

        *cssFS << ".right {\n"
               << "    text-align: right;\n"
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
               << "}\n" << endl;
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
    heading(2, "Deadlock Summary");
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

        *mOFS << "<div id=\"deadlock_" << x + 1 << "\">\n";
        heading(3, "Deadlock " + to_string(x + 1));

        // Open the table.
        *mOFS << "<table  style=\"width:95%\">\n";

        // Line number.
        *mOFS << "<tr><th class=\"right th_small\">Line Number</th>"
              << "<td class=\"number\">"
              << thisDeadlock->lineNumber()
              << "</td></tr>\n";

        // Sessions involved in the deadlock.
        *mOFS << "<tr><th class=\"right th_small\">Sessions involved</th>"
              << "<td class=\"number\">"
              << thisDeadlock->rows()
              << "</td></tr>\n";

        // Deadlock wait reason.
        *mOFS << "<tr><th class=\"right th_small\">Deadlock Wait</th>"
              << "<td id=\"DeadlockWait\">"
              << thisDeadlock->deadlockWait()
              << "</td></tr>\n";

        // Deadlock signature.
        *mOFS << "<tr><th class=\"right th_small\">Deadlock Signature</th>"
              << "<td id=\"DeadlockSignature\">";

              for (unsigned s = 0; s < thisDeadlock->signatures()->size() - 1; x++) {
                  *mOFS << thisDeadlock->signatures()->at(x)
                        << "<br>";
              }

        *mOFS << "</td></tr>\n";

        // Probable cause.
        string signature;
        string reason;
        *mOFS << "<tr><th class=\"right th_small\">Probable Cause</th>"
              << "<td id=\"DeadlockCause\">";

        if (thisDeadlock->ul()) {
            *mOFS << "UL: User defined locking - you are, unfortunately, on your own!<br>\n";
        }

        if (thisDeadlock->txxs()) {
            *mOFS << "TX-X-S: Insufficient ITL entries (docId 1552191.1); or<br>\n"
                  << "TX-X-S: Bitmap indexes (docId 1552175.1); or<br>\n"
                  << "TX-X-S: Manipulation primary/unique key in an inconsistent order (docId 1552191.1).<br>\n";
        }

        if (thisDeadlock->tm()) {
            *mOFS << "TM: Unindexed FK constraint columns (docId 1552169.1).<br>\n";
        }

        if (thisDeadlock->txxx()) {
            if (thisDeadlock->rows() == 1) {
                *mOFS << "TX-X-X: Self deadlock - with an autonomous transaction (docId 1552173); or<br>\n"
                      << "TX-X-X: Self deadlock - without an autonomous transaction (docId 1552123).<br>\n";
            } else {
                *mOFS << "TX-X-X: Deadlock caused by application code (docId 1552120.1).<br>\n";
            }
        }


        *mOFS << "</td></tr>\n";

        // Close the table.
        *mOFS << "</table>\n";

        // Close the div.
        *mOFS << "</div>\n" << endl;
    }
}

//==============================================================================
//                                                                     heading()
//------------------------------------------------------------------------------
// Writes an HTML heading line of a given heading level.
//==============================================================================
void oraDeadlockReport::heading(unsigned level, string heading)
{
    *mOFS << "<h" << level << '>' << heading << "</h" << level << ">\n" << endl;
}

/*
<div id="deadlock_1">
<h3>Deadlock 1</h3>

<table  style="width:95%">
<tr><th class="right th_small">Line Number        </th><td class="number">30</td></tr>
<tr><th class="right th_small">Sessions involved  </th><td class="number">3</td></tr>
<tr><th class="right th_small">Deadlock Wait      </th><td id="DeadlockWait">waiting for 'enq: TX - allocate ITL entry'</td></tr>
<tr><th class="right th_small">Deadlock Signature </th><td id="DeadlockSignature">TX-X-S</td></tr>
<tr><th class="right th_small">Probable Cause     </th><td id="DeadlockCause">BITMAP INDEX/ITL/PK or UK inconsistency Deadlock</td></tr>
</table>
</div>
*/
