bin_PROGRAMS=ftserver
ftserver_SOURCES=daemon.cpp GLog.cpp IOServiceKeep.cpp MessageQueue.cpp NTPClient.cpp handlefs.cpp \
                 FileWritter.cpp \
                 DataTransfer.cpp \
                 TransferClient.cpp TransferAgent.cpp \
                 ftserver.cpp

ftserver_LDFLAGS=-L/usr/local/lib
COMMON_LIBS=-lpthread -lcurl -lm -lrt
BOOST_LIBS=-lboost_system-mt -lboost_thread-mt
ftserver_LDADD=${COMMON_LIBS} ${BOOST_LIBS}
