#!/bin/bash
#-------------------------------------------------------------------------
# A script to scan the PNET01P1 alert log for deadlock entries, from there
# to extract a list of trace file names for said deadlocks. Then, for the
# trace files that still exist, to collect them together in a tar.gz file
# for subsequent SFTPing for further processing.
#-------------------------------------------------------------------------
# You will need to ensure that DIAG is correctly set, either in the shell
# or in this code, to point at where the 'alert' and 'trace' directories
# for the database live, within the diagnostics area.
#
# DIAG=/u01/app/oracle/diag/rdbms/ORACLE_INSTANCE/$ORACLE_SID
#-------------------------------------------------------------------------
# Execution is easy:
#
# 1. Create a working directory.
# 2. Change into it.
# 3. Execute this code.
# 4. Pick up the file $ORACLE_SID_deadlocks.tar.gz.
#-------------------------------------------------------------------------
# History:
#
# 18/09/2018 NDunbar    Created.
# 06/03/2019 NDunbar    Modified to attempt to create DIAG, and validate it
#                       to be sure we are in the right location.
#-------------------------------------------------------------------------


# Variables.
TRACELIST=CollectedTraceFiles.txt
TARFILE=${ORACLE_SID}_deadlocks.tar
ZIPFILE="$TARFILE".gz

# Test DIAG exists
if [ ! -v DIAG ]
then
    # Try to creat DIAG.
    DIAG=${ORACLE_BASE}/diag/rdbms/${ORACLE_SID}/${ORACLE_SID}
fi

echo DIAG currently set to ${DIAG}

# Is DIAG a directory?
if [ !  -d ${DIAG} ]
then
    echo ${DIAG} is not a directory.
    exit 1
fi

# Is it the correct directory?
if [ ! -d ${DIAG}/trace ]
then
    echo ${DIAG} is not an Oracle diagnostic directory.
    exit 1
fi

# Wipe out any existing data.
> "$TRACELIST"
rm "$ZIPFILE" >/dev/null 2>&1


# Scan the alert log for unique deadlock trace file names
echo " "
echo Checking alert log...

grep "^ORA-00060:" ${DIAG}/trace/alert_${ORACLE_SID}.log  |\
awk -F"." '{print $2}'          |\
awk -F" " '{print $5 ".trc"}'   |\
sort -u                         |\
while read traceFile
do
   if [ -f "$traceFile" ]
   then
     echo $traceFile >> "${TRACELIST}"
   else
     echo MISSING: "$traceFile"
   fi
done


# We have a list of existing trace files, collect them together.
echo " "
echo "Collecting ..."

cat "${TRACELIST}" |\
while IFS='' read -r tracefile || [[ -n "$tracefile" ]]
do
     ls -l "$tracefile"
     chmod o+r "$tracefile"
     cp "$tracefile" /tmp/
     chmod o-r "$tracefile"
done

# Create the tar file and gzip it.
echo " "
echo Tarring \(and feathering\) the files...

BACK_THERE=`pwd`
cd /tmp
tar -cf "${BACK_THERE}"/"${TARFILE}" ./${ORACLE_SID}*.trc
rm /tmp/${ORACLE_SID}*.trc
cd "${BACK_THERE}"

echo " "
echo  Zipping "${TARFILE}" ...
gzip -9 "${TARFILE}"


# Finished.
echo " "
echo Finished: \'"${ZIPFILE}"\' is ready for collection.
echo " "

