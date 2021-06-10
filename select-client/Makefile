CC=gcc
TARGET=select-client

all: $(TARGET)

$(TARGET): $(TARGET).o
	$(CC) $^ $(LOADLIBES) $(LDLIBS) -o $@

clean:
	rm -f  *.o $(TARGET)