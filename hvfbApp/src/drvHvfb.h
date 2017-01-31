/*
 * drvHvfb.h
 * Driver for ASTA HV feedback.
 * 12/5/16 md
 */

#include "asynPortDriver.h"


/* These are the drvInfo strings that are used to identify the parameters.
 * They are used by asyn clients, including standard asyn device support */
#define P_RunString                "RUN"                  /* asynInt32,    r/w */
#define P_UpdateTimeString         "UPDATE_TIME"          /* asynFloat64,  r/w */
#define P_IntTimeString            "INT_TIME"             /* asynInt32,    r/w */
#define P_VDesString               "VDES"                 /* asynFloat64,  r/w */
#define P_VMaxString               "VMAX"                 /* asynFloat64,  r/w */
#define P_SmoothingString          "SMOOTHING"            /* asynFloat64,  r/w */
#define P_AveString                "AVE"                  /* asynFloat64,  r/w */
#define P_DiffString               "DIFF"                 /* asynFloat64,  r/w */
#define P_DeltaString              "DELTA"                /* asynFloat64,  r/w */
#define P_LastString               "LAST"                 /* asynFloat64,  r/w */
#define P_CorrectionString         "CORRECTION"           /* asynFloat64,  r/w */
#define P_CFactString              "CFACT"                /* asynFloat64,  r/w */
#define P_InpValString             "INP_VAL"              /* asynFloat64,  w */
#define P_OutNomString             "OUT_NOM"              /* asynFloat64,  w */
#define P_OutValLastString         "OUT_VAL_LAST"         /* asynFloat64,  w */
#define P_OutValString             "OUT_VAL"              /* asynFloat64,  r */

class drvHvfb : public asynPortDriver {
public:
    drvHvfb(const char *portName);
                 
    /* These are the methods that we override from asynPortDriver */
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    virtual asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);

    /* These are the methods that are new to this class */
    void feedbackTask(void);

protected:
    /** Values used for pasynUser->reason, and indexes into the parameter library. */
    int P_Run;
    #define FIRST_COMMAND P_Run
    int P_UpdateTime;
    int P_IntTime;
    int P_VDes;
    int P_VMax;
    int P_Smoothing;
    int P_Ave;
    int P_Diff;
    int P_Delta;
    int P_Last;
    int P_Correction;
    int P_CFact;
    int P_InpVal;
    int P_OutNom;
    int P_OutValLast;
    int P_OutVal;
    #define LAST_COMMAND P_OutVal
 
private:
    /* Our data */
    epicsEventId eventId_;
    double inpVal_;
    double _timedAverage(int intTime);
};


#define NUM_PARAMS (&LAST_COMMAND - &FIRST_COMMAND + 1)

