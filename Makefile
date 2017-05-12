

#MQTTLIB_C = paho-mqtt3c
#MQTTLIB_CS = paho-mqtt3cs
#MQTTLIB_A = paho-mqtt3a
MQTTLIB_AS = paho-mqtt3as
CC = gcc
#CC = arm-linux-gnueabihf-gcc


CFLAGS = -g #-Wall -Wnonnull -Wpointer-sign
OBJECTS = foobar.o mqtt.o serialscomm.o deviceboat.o xprotocol.o 
OBJECTS += config.o cfg_util.o dictionary.o hash.o 
INCFLAGS = -I./
LDFLAGS =
LIBS = -l$(MQTTLIB_AS) 
LIBS += -lpthread 
LIBS += -lyajl_s

ifeq ($(CC),gcc)
CFLAGS += -I/home/kim/usr/x86/mqtt/include
CFLAGS += -I/home/kim/usr/x86/yajl-2.1.1/include
LDFLAGS = -Wl,-rpath,/usr/lib/
LDFLAGS += -L/home/kim/usr/x86/mqtt/lib 
#LDFLAGS += -L/home/kim/usr/x86/openssl/lib
LDFLAGS += -L/home/kim/usr/x86/yajl-2.1.1/lib
export LD_LIBRARY_PATH=/usr/local/lib 
else
CFLAGS += -I/home/kim/usr/arm/yajl-2.1.1/include
CFLAGS += -I/home/kim/usr/arm/mqtt/include
#LDFLAGS = -Wl,-rpath,/home/kim/usr/arm/openssl/lib
LDFLAGS += -L/home/kim/usr/arm/mqtt/lib 
LDFLAGS += -L/home/kim/usr/arm/yajl-2.1.1/lib
endif

all: foobar
foobar: $(OBJECTS)
	$(CC) -o foobar $(OBJECTS) $(LDFLAGS) $(LIBS)

.SUFFIXES:
.SUFFIXES:	.c .cc .C .cpp .o

.c.o :
	$(CC) -o $@ -c $(CFLAGS) $< $(INCFLAGS)

count:
	wc *.c *.cc *.C *.cpp *.h *.hpp

clean:
	rm -f *.o foobar

.PHONY: all
.PHONY: count
.PHONY: clean
