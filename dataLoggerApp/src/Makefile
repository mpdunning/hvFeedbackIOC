TOP=../..

include $(TOP)/configure/CONFIG

PROD_IOC = dataLogger

DBD += dataLogger.dbd
dataLogger_DBD += base.dbd
dataLogger_DBD += asyn.dbd
dataLogger_DBD += asSupport.dbd
dataLogger_DBD += drvDataLogger.dbd

dataLogger_SRCS += dataLogger_registerRecordDeviceDriver.cpp
dataLogger_SRCS += drvDataLogger.cpp
dataLogger_SRCS_DEFAULT += dataLoggerMain.cpp

dataLogger_LIBS += asyn
dataLogger_LIBS += autosave

dataLogger_LIBS += $(EPICS_BASE_IOC_LIBS)

include $(TOP)/configure/RULES
