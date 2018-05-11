include $(SUNSHINE_SRC)/uni_billing/public/env_public.mk
CFLAGS = $(PCFLAGS)
CFLAGS = $(PCFLAGS) -I$(BILL_INC)
LDFLAGS = -L$(PUB_LIB) $(PUBFLAGS)
OBJS = src/common.o src/compiler.o src/database.o src/log_file.o src/array.o \
	src/hashtable.o src/user_exception.o src/ftp.o src/sftp.o src/dispatchtask.o \
	src/ipsocket.o src/memleak.o src/msgqueue.o src/object.o src/semmutex.o \
	src/semrwlock.o src/sharememclient.o src/workthread.o src/udpsocket.o \
	src/crontab.o src/sharememserver.o src/tcpsocket.o src/calculator.o \
	src/simple_reg.o src/threshold.o src/common_hash.o src/perf_stat.o \
	src/dstream.o 
LIBS = libpublic.so

TARGET = $(LIBS)

all:$(TARGET) install

libpublic.so:$(OBJS)
	$(CC) $(CFLAGS) $(SOFLAGS) -o $@ $(OBJS) $(LDFLAGS)
ifndef DEBUG
	-$(STRIP) $@
endif


.SUFFIXES:.o .h
.SUFFIXES:.cpp .o

.cpp.o:
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -f src/*.o $(LIBS)

install:
	-rm -f $(PUB_LIB)/$(LIBS)
	mkdir -p $(PUB_LIB)
	cp $(LIBS) $(PUB_LIB)/

remake: clean all
