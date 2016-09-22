SHARED = -fPIC

CXX = g++ -g -DACE_HAS_EVENT_POLL

DEBUG = -D DEBUG

CINCDIRS = -I. -I../../include -I$(ACE_ROOT)
CLIBDIRS = -L../../lib 
CDEPLIBS = -pthread -ltcp_svc 
OBJS = $(wildcard *.o)
BINARY = $(patsubst %.cpp, %.o, $(wildcard *.cpp))
TARGET = ../../bin/tcp_client 

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
