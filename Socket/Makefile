CC=g++
CFLAGS=-c -Wall -pthread -ggdb -g
LDFLAGS=-pthread -ggdb -g
SOURCES=main.cc TCPClient.cc Packet.cc OFResponder_Test.cc OFSwitchDesc.cc OFPipeline.cc OFPortMng.cc OFPhysicalPort.cc OFTableMng.cc OFTable.cc OFTableFlowEntry.cc
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=main.out

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cc.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o *.out

debug:
	valgrind --leak-check=full --show-leak-kinds=all -v --track-origins=yes ./$(EXECUTABLE) > out.log 2> error.log

run:
	./$(EXECUTABLE) > out.log 2> error.log