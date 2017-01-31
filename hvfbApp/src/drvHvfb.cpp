/*
 * drvHvfb.cpp
 * Driver for ASTA HV feedback.
 * 12/5/16 md
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <iostream>  // std::cout

#include <epicsTypes.h>
#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsString.h>
#include <epicsTimer.h>
#include <epicsMutex.h>
#include <epicsEvent.h>
#include <iocsh.h>

#include "drvHvfb.h"
#include <epicsExport.h>

const double MIN_UPDATE_TIME = 0.02; /* Minimum update time, to prevent CPU saturation */
const int MIN_INT_TIME = 1;    /* Minimum integration time */
const int DEBUG = 0;

static const char *driverName="drvHvfb";
void feedbackTask(void *drvPvt);

/*
  * Constructor for the drvHvfb class.
  * Calls constructor for the asynPortDriver base class.
  * \param[in] portName The name of the asyn port driver to be created.
*/
drvHvfb::drvHvfb(const char *portName) 
   : asynPortDriver(portName, 
                    1, /* maxAddr */ 
                    (int)NUM_PARAMS,
                    asynInt32Mask | asynFloat64Mask | asynDrvUserMask, /* Interface mask */
                    asynInt32Mask | asynFloat64Mask,  /* Interrupt mask */
                    0, /* asynFlags.  This driver does not block and it is not multi-device, so flag is 0 */
                    1, /* Autoconnect */
                    0, /* Default priority */
                    0) /* Default stack size*/    
{
    asynStatus status;
    const char *functionName = "drvHvfb";

    eventId_ = epicsEventCreate(epicsEventEmpty);
    
    createParam(P_RunString,                asynParamInt32,         &P_Run);
    createParam(P_UpdateTimeString,         asynParamFloat64,       &P_UpdateTime);
    createParam(P_IntTimeString,            asynParamInt32,         &P_IntTime);
    createParam(P_VDesString,               asynParamFloat64,       &P_VDes);
    createParam(P_VMaxString,               asynParamFloat64,       &P_VMax);
    createParam(P_SmoothingString,          asynParamFloat64,       &P_Smoothing);
    createParam(P_AveString,                asynParamFloat64,       &P_Ave);
    createParam(P_DiffString,               asynParamFloat64,       &P_Diff);
    createParam(P_DeltaString,              asynParamFloat64,       &P_Delta);
    createParam(P_LastString,               asynParamFloat64,       &P_Last);
    createParam(P_CorrectionString,         asynParamFloat64,       &P_Correction);
    createParam(P_CFactString,              asynParamFloat64,       &P_CFact);
    createParam(P_InpValString,             asynParamFloat64,       &P_InpVal);
    createParam(P_OutNomString,             asynParamFloat64,       &P_OutNom);
    createParam(P_OutValLastString,         asynParamFloat64,       &P_OutValLast);
    createParam(P_OutValString,             asynParamFloat64,       &P_OutVal);
    
    /* Set the initial values of some parameters */
    setIntegerParam(P_Run,                 0);
    setIntegerParam(P_IntTime,             6);
    setDoubleParam(P_UpdateTime,         0.2);
    setDoubleParam(P_Smoothing,          0.5);
    setDoubleParam(P_CFact,            74.35);
    setDoubleParam(P_OutNom,             3.0);
    setDoubleParam(P_VDes,              -265);
    setDoubleParam(P_VMax,              -240);
    setDoubleParam(P_Ave,                0.0);
    setDoubleParam(P_Diff,               0.0);
    setDoubleParam(P_Delta,              0.0);
    setDoubleParam(P_Last,               0.0);
    setDoubleParam(P_Correction,         0.0);
    setDoubleParam(P_OutVal,             0.0);
    
    /* Create the feedback thread in the background */
    status = (asynStatus)(epicsThreadCreate("drvHvfbTask",
                          epicsThreadPriorityMedium,
                          epicsThreadGetStackSize(epicsThreadStackMedium),
                          (EPICSTHREADFUNC)::feedbackTask,
                          this) == NULL);
    if (status) {
        printf("%s:%s: epicsThreadCreate failure\n", driverName, functionName);
        return;
    }
}

void feedbackTask(void *drvPvt)
{
    drvHvfb *pPvt = (drvHvfb *)drvPvt;
    pPvt->feedbackTask();
}

// Feedback task that runs as a separate thread.
void drvHvfb::feedbackTask(void)
{
    /* This thread computes the waveform and does callbacks with it */

    double updateTime, smoothing;
    double vDes, vMax, ave;
    double diff, delta, cFact;
    double nom, last, correction;
    int run, intTime;
    using std::cout;
    using std::endl;
    
    lock();
    /* Loop forever */    
    while (1) {
        getDoubleParam(P_UpdateTime, &updateTime);
        getIntegerParam(P_Run, &run);
        // Release the lock while we wait for a command to start or wait for updateTime
        unlock();
        if (run) epicsEventWaitWithTimeout(eventId_, updateTime);
        else     (void) epicsEventWait(eventId_);
        // Take the lock again
        lock(); 
        /* run could have changed while we were waiting */
        getIntegerParam(P_Run, &run);
        if (!run) continue;
        getIntegerParam(P_IntTime, &intTime);
        getDoubleParam(P_VMax, &vMax);
        getDoubleParam(P_OutNom, &nom);
        // Release lock to see changes while computing average
        unlock();
        ave = _timedAverage(intTime);
        // Take the lock again
        lock();

        if (ave < vMax) {
            getDoubleParam(P_VDes, &vDes);
            getDoubleParam(P_CFact, &cFact);
            getDoubleParam(P_OutValLast, &last);
            getDoubleParam(P_Smoothing, &smoothing);
            diff = vDes - ave;
            delta = diff/cFact;
            correction = last - delta*smoothing;
            setDoubleParam(P_OutVal, correction);
        } else {
            setDoubleParam(P_OutVal, nom);
        }

        if (DEBUG) {
            cout << "ave=" << ave << ", ";
            cout << "cFact=" << cFact << ", ";
            cout << "last=" << last << ", ";
            cout << "diff=" << diff << ", ";
            cout << "delta=" << delta << endl;
        }

        setDoubleParam(P_Ave, ave);
        setDoubleParam(P_Diff, diff);
        setDoubleParam(P_Delta, delta);
        setDoubleParam(P_Last, last);
        setDoubleParam(P_Correction, correction);
        
        callParamCallbacks();
    }
}

double drvHvfb::_timedAverage(int intTime) {
    const char *functionName = "_timedAverage";
    double sum = 0;
    double time, act, ave;
    using std::cout;
    using std::endl;

    for (time=0; time<intTime; time++) {
        getDoubleParam(P_InpVal, &inpVal_);
        act = inpVal_;
        sum += act;
        epicsThreadSleep(1);
    
        if (DEBUG) {
            cout << functionName << ": act=" << act << ", time= " << time << ", sum=" << sum << endl;
        }
    }
    
    ave = sum/time;
   
    if (DEBUG) {
        cout << functionName << ": ave=" << ave << endl;
    }
    
    return ave;
}

/** Called when asyn clients call pasynInt32->write().
  * This function sends a signal to the feedbackTask thread if the value of P_Run has changed.
  * For all parameters it sets the value in the parameter library and calls any registered callbacks..
  * \param[in] pasynUser pasynUser structure that encodes the reason and address.
  * \param[in] value Value to write. */
asynStatus drvHvfb::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    int function = pasynUser->reason;
    asynStatus status = asynSuccess;
    const char *paramName;
    const char* functionName = "writeInt32";

    /* Set the parameter in the parameter library. */
    status = (asynStatus) setIntegerParam(function, value);
    
    /* Fetch the parameter string name for possible use in debugging */
    getParamName(function, &paramName);

    if (function == P_Run) {
        /* If run was set then wake up the simulation task */
        if (value) epicsEventSignal(eventId_);
    } 
    else if (function == P_IntTime) {
        if (value < MIN_INT_TIME) {
            asynPrint(pasynUser, ASYN_TRACE_WARNING,
                "%s:%s: warning, integration time too small, changed from %d to %d\n", 
                driverName, functionName, value, MIN_INT_TIME);
            value = MIN_INT_TIME;
        }
        status = (asynStatus) setIntegerParam(function, value);
    }
    else {
        /* All other parameters just get set in parameter list, no need to
         * act on them here */
    }
    
    /* Do callbacks so higher layers see any changes */
    status = (asynStatus) callParamCallbacks();
    
    if (status) 
        epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, 
                  "%s:%s: status=%d, function=%d, name=%s, value=%d", 
                  driverName, functionName, status, function, paramName, value);
    else        
        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
              "%s:%s: function=%d, name=%s, value=%d\n", 
              driverName, functionName, function, paramName, value);
    return status;
}

/** Called when asyn clients call pasynFloat64->write().
  * This function sends a signal to the feedbackTask thread if the value of P_UpdateTime has changed.
  * For all  parameters it  sets the value in the parameter library and calls any registered callbacks.
  * \param[in] pasynUser pasynUser structure that encodes the reason and address.
  * \param[in] value Value to write. */
asynStatus drvHvfb::writeFloat64(asynUser *pasynUser, epicsFloat64 value)
{
    int function = pasynUser->reason;
    asynStatus status = asynSuccess;
    int run;
    const char *paramName;
    const char* functionName = "writeFloat64";

    /* Set the parameter in the parameter library. */
    status = (asynStatus) setDoubleParam(function, value);

    /* Fetch the parameter string name for possible use in debugging */
    getParamName(function, &paramName);

    if (function == P_UpdateTime) {
        /* Make sure the update time is valid. If not change it and put back in parameter library */
        if (value < MIN_UPDATE_TIME) {
            asynPrint(pasynUser, ASYN_TRACE_WARNING,
                "%s:%s: warning, update time too small, changed from %f to %f\n", 
                driverName, functionName, value, MIN_UPDATE_TIME);
            value = MIN_UPDATE_TIME;
            setDoubleParam(P_UpdateTime, value);
        }
        /* If the update time has changed and we are running then wake up the simulation task */
        getIntegerParam(P_Run, &run);
        if (run) epicsEventSignal(eventId_);
    } else {
        /* All other parameters just get set in parameter list, no need to
         * act on them here */
    }
    
    /* Do callbacks so higher layers see any changes */
    status = (asynStatus) callParamCallbacks();
    
    if (status) 
        epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, 
                  "%s:%s: status=%d, function=%d, name=%s, value=%f", 
                  driverName, functionName, status, function, paramName, value);
    else        
        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
              "%s:%s: function=%d, name=%s, value=%f\n", 
              driverName, functionName, function, paramName, value);
    return status;
}

/* Configuration routine.  Called directly, or from the iocsh function below */

extern "C" {
/*
* EPICS iocsh callable function to call constructor for the drvHvfb class.
* \param[in] portName The name of the asyn port driver to be created.
*/
int drvHvfbConfigure(const char *portName)
{
    new drvHvfb(portName);
    return(asynSuccess);
}


/* EPICS iocsh shell commands */

static const iocshArg initArg0 = {"portName", iocshArgString};
static const iocshArg * const initArgs[] = {&initArg0};
static const iocshFuncDef initFuncDef = {"drvHvfbConfigure", 1, initArgs};
static void initCallFunc(const iocshArgBuf *args)
{
    drvHvfbConfigure(args[0].sval);
}

void drvHvfbRegister(void)
{
    iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(drvHvfbRegister);

}

