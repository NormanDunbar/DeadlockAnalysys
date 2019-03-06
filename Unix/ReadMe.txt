= CollectDeadlockTraces.sh
Norman Dunbar <norman@dunbar-it.co.uk>
:revnumber: v1.0
:revdate: March 6 2019


*What is it?*

CollectDeadlockTraces.sh is a script to run under a bash shell to:


*What does it do?*

* Locate the alert.log for the current Oracle database environment;
* Scan it for deadlocks - ORA-00060 - and obtain the created trace file's name;
* List any trace files which are not found in the diagnostics area;
* Extract a list of deadlock trace files which still exist;
* Tar them all together;
* Compress the resulting tar file with gzip.


*How do I run it?*

Just execute it. You might need to allow execution by:

    chmod ug+x collectDeadlockTraces.sh
    
Now you can execute it!    


*Give me an example*

A test run looks remarkably like the following, for a system that suffered badly from deadlocks caused by using bitmap indexes on an OLTP system, Sigh!

[source]
----
$ ./collectDeadlockTraces.sh

DIAG currently set to /u01/app/oracle/diag/rdbms/xxxxxxxx/xxxxxxxx

Checking alert log...
MISSING: /u01/app/oracle/diag/rdbms/xxxxxxxx/xxxxxxxx/trace/xxxxxxxx_ora_10158780.trc
...
...
MISSING: /u01/app/oracle/diag/rdbms/xxxxxxxx/xxxxxxxx/trace/xxxxxxxx_ora_9175774.trc

Collecting ...
-rw-r-----    1 oracle   asmadmin    1694112 Mar  2 06:05 /u01/app/oracle/diag/rdbms/xxxxxxxx/xxxxxxxx/trace/xxxxxxxx_ora_14025328.trc
-rw-r-----    1 oracle   asmadmin    1515803 Mar  2 06:05 /u01/app/oracle/diag/rdbms/xxxxxxxx/xxxxxxxx/trace/xxxxxxxx_ora_15860432.trc
...
...
-rw-r-----    1 oracle   asmadmin    1897975 Feb 11 10:55 /u01/app/oracle/diag/rdbms/xxxxxxxx/xxxxxxxx/trace/xxxxxxxx_ora_7602800.trc
-rw-r-----    1 oracle   asmadmin    1147204 Feb 27 09:34 /u01/app/oracle/diag/rdbms/xxxxxxxx/xxxxxxxx/trace/xxxxxxxx_ora_9437892.trc

Tarring (and feathering) the files...

Zipping xxxxxxxx_deadlocks.tar ...

Finished: 'xxxxxxxx_deadlocks.tar.gz' is ready for collection.
----

*OK, now what?*

* Download the generated `xxxxxxxx_deadlocks.tar.gz` file;
* Gunzip it (On Windows, https://www.7-zip.org/download.html[`7zip`] is useful for this);
* Untar it (https://www.7-zip.org/download.html[`7zip`] again);
* Run `DeadlockAnalysis` against the resulting pile of trace files;
* Open the generated HTML files in your favourite browser, or use Internet Explorer, and examine why the deadlocks occurred.


*What are all those Xs about?*

They replace the `instance name` and `Oracle_SID` for the database I was testing this code on. It's a small way to protect the innocent and/or guilty and to avoid database names escaping into the wilds where miscreants _might_ wish to attack them.

*Enjoy*.


