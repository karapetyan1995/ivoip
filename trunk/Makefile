# makefile for iPhone

CC=arm-apple-darwin-cc 
LD=$(CC) 
LDFLAGS=-framework CoreAudio -framework AudioToolbox -framework CoreFoundation -framework Foundation  -framework Celestial -framework MeCCA

all: $(TRG)

$(TRG): $(TRG).o
	$(LD) $(LDFLAGS) -v -o $@ $^ 

%.o: %.c

	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@ 

clean: 

	rm -f *.o $(TRG)
