#!../../bin/linux-x86_64/hvfb

< envPaths

epicsEnvSet("P",         "ASTA:HVFB01")
epicsEnvSet("DESC",      "ASTA...")
epicsEnvSet("PORT",      "HVFB")
epicsEnvSet("INPUT_PV",  "KLYS:AS01:K1:2:SACTUAL")
epicsEnvSet("OUTPUT_PV", "ASTA:AO:4132-32:CH1")

cd ${TOP}

dbLoadDatabase("dbd/hvfb.dbd",0,0)
hvfb_registerRecordDeviceDriver(pdbbase)

save_restoreSet_status_prefix("")
save_restoreSet_IncompleteSetsOk(1)
save_restoreSet_DatedBackupFiles(1)
set_savefile_path("/nfs/slac/g/testfac/asta/$(IOC)", "autosave")
set_pass0_restoreFile("hvfb.sav")
set_pass1_restoreFile("hvfb.sav")

# drvHvfbConfigure(char* port)
#-----------------------------------------------------
drvHvfbConfigure("$(PORT)")

#asynSetTraceMask("$(PORT)", -1, 0x0f)
#asynSetTraceIOMask("$(PORT)", -1, 0x6)

dbLoadRecords("db/hvfb.db", "P=$(P), PORT=$(PORT), IOC=$(IOC), DESC=$(DESC), INPUT_PV=$(INPUT_PV), OUTPUT_PV=$(OUTPUT_PV)")

cd ${TOP}/iocBoot/${IOC}
iocInit()

create_monitor_set("hvfb.req", 30, "P=$(P)")


