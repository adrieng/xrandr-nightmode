CFLAGS=-Wall -O2
LDFLAGS=-lxcb-randr -lxcb

PREFIX=$(HOME)
BINDIR=$(PREFIX)/bin

TARGET=xrandr-nightmode.bin

all: $(TARGET)

%.bin: gamma_randr.o %.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

%.o: %.h

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) *.o

install: $(TARGET)
	install -v -s -m 755 $(TARGET) $(BINDIR)/$(shell basename $(TARGET) .bin)

uninstall:
	rm -f $(BINDIR)/$(shell basename $(TARGET) .bin)
