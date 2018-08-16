SOURCE  := $(wildcard *.c) $(wildcard *.cpp)
OBJS    := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))

TARGET  := volume_ctrl

CC      := gcc
CXX     := g++
DEFINES := 
INCLUDE :=

CFLAGS  += -g -Wall -O3 $(DEFINES) $(INCLUDE) `pkg-config alsa --cflags`
LIBS    += `pkg-config alsa --libs`

LDFLAGS :=
CXXFLAGS:= $(CFLAGS) -DHAVE_CONFIG_H
  
.PHONY : everything objs clean distclean rebuild
  
all : $(TARGET)
  
objs : $(OBJS)
  
rebuild: distclean all
                
clean :
	rm -rf *~
	rm -rf *.o
    
distclean : clean
	rm -rf $(TARGET)
  
$(TARGET) : $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)
