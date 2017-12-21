CXXFLAGS =	-O2 -g -Wall -pthread  -std=c++11 -fmessage-length=0 -I/home/arthur/sfpython/boost_1_65_1 -I.

OBJS =		DataProcessor.o

LIBS =  -lpthread

TARGET =	processor

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	clean $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
