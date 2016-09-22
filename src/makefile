SHARED = -fPIC -shared

CXX = g++ -g -DACE_HAS_EVENT_POLL

DEBUG = -D DEBUG

CINCDIRS = -I. -I$(ACE_ROOT) 
CLIBDIRS = -L$(ACE_ROOT)/lib 
CDEPLIBS = -pthread -lACE 
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
	cp $(TARGET) ../lib
