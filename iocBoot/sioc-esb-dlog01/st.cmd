#!../../bin/linux-x86_64/dataLogger

< envPaths

epicsEnvSet("P",    "ESB:DLOG01")
epicsEnvSet("DESC", "Test 1")
epicsEnvSet("PORT", "DLOG")
epicsEnvSet(EPICS_CA_MAX_ARRAY_BYTES, 100000)

cd ${TOP}

dbLoadDatabase("dbd/dataLogger.dbd",0,0)
dataLogger_registerRecordDeviceDriver(pdbbase)

# drvDataLoggerConfig(char* port, int npvs)
#-----------------------------------------------------
drvDataLoggerConfig("$(PORT)", "3")

#asynSetTraceMask("$(PORT)", -1, 0x0f)
#asynSetTraceIOMask("$(PORT)", -1, 0x6)

dbLoadRecords("db/dlog.db", "P=$(P), PORT=$(PORT), DESC=$(DESC)")

cd ${TOP}/iocBoot/${IOC}
iocInit()

epicsThreadSleep(1)
dbpf "$(P):FILEPATH" "/nfs/slac/g/nlcta/u01/nlcta/pvLog/test/"
dbpf "$(P):COMMENT" "Comment"
