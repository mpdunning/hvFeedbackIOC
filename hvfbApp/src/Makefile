TOP=../..

include $(TOP)/configure/CONFIG

PROD_IOC = hvfb

DBD += hvfb.dbd
hvfb_DBD += base.dbd
hvfb_DBD += asyn.dbd
hvfb_DBD += asSupport.dbd
hvfb_DBD += drvHvfb.dbd

hvfb_SRCS += hvfb_registerRecordDeviceDriver.cpp
hvfb_SRCS += drvHvfb.cpp
hvfb_SRCS_DEFAULT += hvfbMain.cpp

hvfb_LIBS += asyn
hvfb_LIBS += autosave

hvfb_LIBS += $(EPICS_BASE_IOC_LIBS)

include $(TOP)/configure/RULES
