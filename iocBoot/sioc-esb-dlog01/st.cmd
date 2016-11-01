#!../../bin/linux-x86_64/dataLogger

< envPaths

epicsEnvSet("P",    "ESB:DLOG01")
epicsEnvSet("DESC", "Test 1")
epicsEnvSet("PORT", "DLOG")
epicsEnvSet(EPICS_CA_MAX_ARRAY_BYTES, 100000)

cd ${TOP}

dbLoadDatabase("dbd/dataLogger.dbd",0,0)
dataLogger_registerRecordDeviceDriver(pdbbase)

save_restoreSet_status_prefix("")
save_restoreSet_IncompleteSetsOk(1)
save_restoreSet_DatedBackupFiles(1)
set_savefile_path("/nfs/slac/g/testfac/esb/$(IOC)", "autosave")
set_pass0_restoreFile("dlog.sav")
set_pass1_restoreFile("dlog.sav")

# drvDataLoggerConfig(char* port, int npvs)
#-----------------------------------------------------
drvDataLoggerConfig("$(PORT)", "3")

#asynSetTraceMask("$(PORT)", -1, 0x0f)
#asynSetTraceIOMask("$(PORT)", -1, 0x6)

dbLoadRecords("db/dlog.db", "P=$(P), PORT=$(PORT), DESC=$(DESC)")

cd ${TOP}/iocBoot/${IOC}
iocInit()

# Default values
dbpf $(P):FILEPATH "/nfs/slac/g/nlcta/u01/nlcta/pvLog/test/"
dbpf $(P):COMMENT  "Comment"

create_monitor_set("dlog.req", 30, "P=$(P)")

epicsThreadSleep(0.5)

# This should restore from the autosave file after any dbpf commands
fdbrestore("dlog.sav")

