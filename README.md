# DeadlockAnalysys
An attempt to extract useful information and diagnostics from an Oracle trace file containing deadlock details. (*And, yes, I do wish I had spelt 'analysis' correctly when I set up this repository!*)

## Introduction
The very first release of this application which has languished, unloved, since some time in 2017 when I first started it!

### Binaries & Source Code
There are compiled versions for Windows 32/64 bit (`DeadlockAnalysis.exe`) and Linux 32/64 bit too (`DeadlockAnalysis`). Source code is available to compile of other systems which I don't have. GCC was used to create this utility. You will also find a Code::Blocks project file if you use that IDE.

### What is it?
This utility will read a trace file produced by the Oracle database and scan it for a deadlock, or more than one if that's what it finds. For each deadlock it will generate a report with the relevant details of the deadlock extracted from all the cruft in the trace file.

Where it can, it tries to diagnose the underlying reason as to why the deadlock occurred. These, currently, being:

* User defined locks - `select ... for update`, `lock table ... in exclusive mode`, use of `dbms_lock` etc;
* Application caused deadlocks - getting hold of resources in different orders in different sessions;
* Self-deadlocks - use and misuse of autonomous transactions, for example;
* Misuse of Primary or Unique Keys - multiple sessions trying to write to the same PK/UK;
* Misuse of bitmap indexes - there's more to these indexes than meets the eye;
* Insufficient ITL entries;

### Execution
Run the utility with a list of Oracle trace files on the command line. Each trace gets a new report file, in the same folder as the trace file.

### Reports
The report is in HTML format and there will be a single report file for each trace file passed. There is a separate CSS file to format the report. You can edit this to suit your own installation standards - it will not be overwritten if it exists when the utility is run.

Have fun diagnosing the reasons for your Oracle Deadlocks.
