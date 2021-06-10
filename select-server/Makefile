CC=gcc
TARGET=select-server

all: $(TARGET)

$(TARGET): $(TARGET).o
	$(CC) $^ $(LOADLIBES) $(LDLIBS) -o $@

clean:
	rm -f  *.o $(TARGET)