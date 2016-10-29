/*
drvDataLogger.cpp
*/

#include <iostream>     // std::cout

//#include <cstdlib>
#include <cstring>
//#include <stdio.h>
//#include <sys/stat.h>
//#include <fcntl.h>
//#include <unistd.h>
//#include <errno.h>

#include <epicsTime.h>
#include <epicsThread.h>
//#include <cantProceed.h>
//#include <initHooks.h>
#include <epicsExport.h>
//#include <errlog.h>
#include <iocsh.h>


#include "drvDataLogger.h"

static const char* driverName="drvDataLogger";


// Constructor for the drvDataLogger class
drvDataLogger::drvDataLogger(const char* port, int npvs)
    : asynPortDriver(port, 1, N_PARAMS,
	asynInt32Mask | asynOctetMask | asynFloat64Mask | asynDrvUserMask,
	asynInt32Mask | asynOctetMask | asynFloat64Mask,
	ASYN_CANBLOCK, // asyn Flags
    1, 0, 0), // Autoconnect, Default priority, Default stack size 
    npvs_(npvs)
{

    //const char *functionName = "drvDataLogger";

    createParam(data0Str,       asynParamFloat64,   &data0);
    createParam(data1Str,       asynParamFloat64,   &data1);
    createParam(data2Str,       asynParamFloat64,   &data2);
    createParam(data3Str,       asynParamFloat64,   &data3);
    createParam(data4Str,       asynParamFloat64,   &data4);
    createParam(data5Str,       asynParamFloat64,   &data5);
    createParam(data6Str,       asynParamFloat64,   &data6);
    createParam(data7Str,       asynParamFloat64,   &data7);
    createParam(data8Str,       asynParamFloat64,   &data8);
    createParam(data9Str,       asynParamFloat64,   &data9);
    createParam(runStateStr,    asynParamInt32,     &runState);
    createParam(runStatusStr,   asynParamInt32,     &runStatus);
    createParam(triggerStr,     asynParamInt32,     &trigger);
    createParam(trigCntStr,     asynParamInt32,     &trigCnt);
    createParam(filepathStr,    asynParamOctet,     &filepath);
    createParam(filenameStr,    asynParamOctet,     &filename);
    createParam(commentStr,     asynParamOctet,     &comment);
    createParam(messageStr,     asynParamOctet,     &message);

    data_ = new double [npvs_];

}

void drvDataLogger::_openFile() {
    const char *functionName = "_openFile";
    using namespace std;
    char fname[64];
    char fullFilename[128];
    char fileExt[] = ".dat";
    char commentBuf[128];
    
    epicsTimeGetCurrent(&timeStamp_);
    epicsTimeToStrftime(timeString_, sizeof(timeString_), "%Y%m%d_%H%M%S", &timeStamp_); 
    
    strncpy(fname, timeString_, sizeof(fname));
    fname[sizeof(fname) - 1] = '\0';
    strncat(fname, fileExt, sizeof(fileExt));
    strncpy(fullFilename, filepath_, sizeof(fullFilename)); 
    fullFilename[sizeof(fullFilename) - 1] = '\0';
    strncat(fullFilename, fname, sizeof(fname));
    outfile_.open(fullFilename);
   
    triggerCount_ = 0;
    setIntegerParam(trigCnt, triggerCount_);
    
    if (outfile_.is_open()) {
        cout << driverName << "::" << functionName << ": Opened file:" << endl;
        cout << fullFilename << endl;
        setStringParam(message, "Opened file");
        getStringParam(comment, sizeof(commentBuf), commentBuf);
        outfile_ << commentBuf << endl;
        outfile_ << "================================================" << endl;
        setIntegerParam(runStatus, running_);
        setStringParam(filename, fname);
    } else {
        cout << driverName << "::" << functionName << ": Could not open file." 
        << endl;
        setStringParam(message, "Could not open file");
    }
    callParamCallbacks();
}

void drvDataLogger::_closeFile() {
    const char *functionName = "_closeFile";
    using namespace std;
    
    if (outfile_.is_open()) {
        outfile_.close();
        setStringParam(message, "Closed file");
    }
    if (!outfile_.is_open()) {
        cout << driverName << "::" << functionName << ": File closed." << endl;
        setIntegerParam(runStatus, running_);
    }
    callParamCallbacks();
}

void drvDataLogger::_writeData() {
    const char *functionName = "_writeData";
    using namespace std;
    int i;

    epicsTimeGetCurrent(&timeStamp_);
    //cout << timeStamp_.secPastEpoch << "." << timeStamp_.nsec << endl;
    epicsTimeToStrftime(timeString_, sizeof(timeString_), "%Y%m%d_%H%M%S.%06f", &timeStamp_); 
    //cout << timeString << endl;

    cout << functionName << ": npvs_: " << npvs_ << endl;
    
    if (outfile_.is_open()) {
        outfile_ << timeString_ << " ";
        outfile_ << fixed;
        for (i=0; i<npvs_; i++) {
            outfile_ << data_[i] << " ";
        }
        outfile_ << endl;
    } else {
        cout << driverName << "::" << functionName << ": Error writing data." << endl;
    }
}

asynStatus drvDataLogger::writeInt32(asynUser* pasynUser, epicsInt32 value) {
    const char* functionName = "writeInt32";
    int function = pasynUser->reason;
    asynStatus status = asynSuccess;
    int addr = 0;
    //const char *paramName;
    using namespace std;

    status = (asynStatus)setIntegerParam(function, value);
    
    if (function == runState) {
        getIntegerParam(runState, &running_);
        cout << driverName << "::" << functionName << ": running_=" << running_ << endl;
        if (running_ && !outfile_.is_open()) {
            _openFile();
        } else if (!running_ && outfile_.is_open()) {
            _closeFile();
        } else { 
            return asynError;
        }
    } else if (function == trigger) {
        if (running_ && outfile_.is_open()) {
            _writeData();
            ++triggerCount_;
            cout << driverName << "::" << functionName << ": Got trigger, trig cnt=" << triggerCount_ << endl;
            setIntegerParam(trigCnt, triggerCount_);
        } 
    }
    
    status = (asynStatus)setIntegerParam(function, value);
    status = (asynStatus)callParamCallbacks();

    if (status) { 
        asynPrint(pasynUser, ASYN_TRACE_ERROR, 
                 "%s:%s, port %s, ERROR writing %d to address %d, status=%d\n",
                 driverName, functionName, portName, value, addr, status);
    } else {        
        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
                 "%s:%s, port %s, wrote %d to address %d\n",
                 driverName, functionName, portName, value, addr);
    }
    
    return status;
}

asynStatus drvDataLogger::writeFloat64(asynUser *pasynUser, epicsFloat64 value) {
    const char* functionName = "writeFloat64";
    int function = pasynUser->reason;
    asynStatus status = asynSuccess;
    int addr = 0;
    using namespace std;

    status = (asynStatus)setDoubleParam(function, value);

    cout << fixed;
    cout << driverName << "::" << functionName << ": function=" << function 
        << ", value=" << value << endl;

    if (function == data0) {
        getDoubleParam(function, &data_[0]);
    } else if (function == data1) {
        getDoubleParam(function, &data_[1]);
    } else if (function == data2) {
        getDoubleParam(function, &data_[2]);
    } else if (function == data3) {
        getDoubleParam(function, &data_[3]);
    } else if (function == data4) {
        getDoubleParam(function, &data_[4]);
    } else if (function == data5) {
        getDoubleParam(function, &data_[5]);
    } else if (function == data6) {
        getDoubleParam(function, &data_[6]);
    } else if (function == data7) {
        getDoubleParam(function, &data_[7]);
    } else if (function == data8) {
        getDoubleParam(function, &data_[8]);
    } else if (function == data9) {
        getDoubleParam(function, &data_[9]);
    }
    
    status = (asynStatus)setDoubleParam(function, value);
    status = (asynStatus)callParamCallbacks();
    
    if (status) { 
        asynPrint(pasynUser, ASYN_TRACE_ERROR, 
                 "%s:%s, port %s, ERROR writing %f to address %d, status=%d\n",
                 driverName, functionName, portName, value, addr, status);
    } else {        
        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
                 "%s:%s, port %s, wrote %f to address %d\n",
                 driverName, functionName, portName, value, addr);
    }
    
    return status;
}

asynStatus drvDataLogger::writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual) {
    const char* functionName = "writeOctet";
    int function = pasynUser->reason;
    asynStatus status = asynSuccess;
    int addr = 0;

    //printf("%s::%s: function=%d (%s), addr=%d, value=%s, nChars=%d, nActual=%d\n", 
    //    driverName, functionName, function, paramName, addr, value, (int)nChars, (int)*nActual);
    
    if (function == filepath) {
        strcpy(filepath_, value);
        *nActual = nChars;
    } else {
        /* All other parameters just get set in parameter list, no need to
         * act on them here */
    }
    
    status = (asynStatus) setStringParam(function, value);
    status = (asynStatus) callParamCallbacks();
    
    if (status) { 
        asynPrint(pasynUser, ASYN_TRACE_ERROR, 
                 "%s:%s, port %s, ERROR writing %s to address %d, status=%d\n",
                 driverName, functionName, portName, value, addr, status);
    } else {        
        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
                 "%s:%s, port %s, wrote %s to address %d\n",
                 driverName, functionName, portName, value, addr);
    }
    
    return status;
}


// Configuration routines.  Called directly, or from the iocsh function below
extern "C" {

int drvDataLoggerConfig(const char* port, int npvs) {
/*-----------------------------------------------------------------------------
 * EPICS iocsh callable function to call constructor for the drvDataLogger class.
 *  port The name of the asyn port driver to be created.
 *  npvs is the number of sampled PVs.
 *---------------------------------------------------------------------------*/
    new drvDataLogger(port, npvs);
    return(asynSuccess);
}

/* EPICS iocsh shell commands */
static const iocshArg initArg0 = { "port", iocshArgString};
static const iocshArg initArg1 = { "npvs", iocshArgInt};
static const iocshArg * const initArgs[] = {&initArg0, &initArg1};
static const iocshFuncDef initFuncDef = {"drvDataLoggerConfig", 2, initArgs};
static void initCallFunc(const iocshArgBuf *args) {
    drvDataLoggerConfig(args[0].sval, args[1].ival);
}

void drvDataLoggerRegister(void) {
    iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(drvDataLoggerRegister);
}


