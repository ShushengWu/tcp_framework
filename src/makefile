SHARED = -fPIC -shared

CXX = g++ -g -DACE_HAS_EVENT_POLL

DEBUG = -D DEBUG

CINCDIRS = -I. -I$(ACE_ROOT) \
           -I/usr/include/mysql
CLIBDIRS = -L$(ACE_ROOT)/lib \
           -L/usr/lib64/mysql
CDEPLIBS = -pthread -lACE -lmysqlclient
OBJS = $(wildcard *.o)
BINARY = $(patsubst %.cpp, %.o, $(wildcard *.cpp))
TARGET = libtcp_svc.so 

all:$(TARGET)
debug: CXX += $(DEBUG)
debug: all

$(TARGET):$(BINARY)
	$(CXX) $? -o $@ $(CLIBDIRS) $(CDEPLIBS)  $(SHARED)
%.o:%.cpp
	$(CXX) -c $< -o $@ $(CINCDIRS) $(SHARED)

.PHONY : clean
clean:
	-rm $(OBJS) $(TARGET)

.PHONY : install
install:
	cp processor.h ../include
	cp log.h ../include
	cp confreader.h ../include
	cp mysqlconn.h ../include
	cp $(TARGET) ../lib
