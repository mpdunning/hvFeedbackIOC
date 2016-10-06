/*
drvDataLogger.h
*/

#include <fstream>      // std::ofstream

#include "asynPortDriver.h"

#define data0Str	    "DATA0" 
#define data1Str	    "DATA1"	
#define data2Str	    "DATA2"	
#define runStateStr	    "RUN_STATE"
#define runStatusStr	"RUN_STATUS"
#define triggerStr	    "TRIGGER"
#define trigCntStr	    "TRIG_CNT"
#define filepathStr	    "FILEPATH"
#define filenameStr	    "FILENAME"
#define commentStr	    "COMMENT"
#define messageStr	    "MESSAGE"

class drvDataLogger : public asynPortDriver {
public:
    drvDataLogger(const char* port, int npvs);
    //virtual asynStatus readInt32(asynUser* pasynUser, epicsInt32* value);
    virtual asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t maxChars, size_t *nActual);
    virtual asynStatus writeInt32(asynUser* pasynUser, epicsInt32 value);
    virtual asynStatus writeFloat64(asynUser* pasynUser, epicsFloat64 value);
    //virtual asynStatus getTimeStamp(epicsTimeStamp *pTimeStamp);

protected:
    int data0;
    int data1;
    int data2;
    int runState;
    int runStatus;
    int trigger;
    int trigCnt;
    int filepath;
    int filename;
    int comment;
    int message;
    #define FIRST_COMMAND data0
    #define LAST_COMMAND message
    #define N_PARAMS ((int)(&LAST_COMMAND - &FIRST_COMMAND + 1))

private:
    asynUser *pasynUser;
    void _openFile();
    void _closeFile();
    void _writeData();
    static const int npvs_ = 3;
    double data_[npvs_];
    int running_;
    std::ofstream outfile_;
    epicsTimeStamp timeStamp_;
    char timeString_[80];
    char filepath_[128];
    int triggerCount_;
};

