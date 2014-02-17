DBUS_CFLAGS = `pkg-config --cflags dbus-1`
DBUS_LIBS = `pkg-config --libs dbus-1`

INC = -I/opt/java/include -I/opt/java/include/linux

CFLAGS = -fPIC
BITNESS = -m64
ARCHDIR = x86_64

ifeq '$(ARCH)' '32'
	BITNESS = -m32
	ARCHDIR = x86
endif

test: CFLAGS += -DTEST

all: libsecretServiceDBus.so

x64: BITNESS = -m64

clean:
	rm -f *.so *.o "test"

dist:
	make install
	make ARCH=32 install

install: clean all
	mv libsecretServiceDBus.so  ../lib-c/$(ARCHDIR)

test: secretServiceDBus-test.o secretServiceDBus.o
	$(CC) $(BITNESS) $(CFLAGS) $(DBUS_LIBS) $^ -o $@
	./test

libsecretServiceDBus.so: secretServiceDBus-jni.o secretServiceDBus.o
	$(CC) $(BITNESS) -shared $(CFLAGS) $(DBUS_LIBS) $^ -o $@

%.o: %.c
	$(CC) $(BITNESS) -c -shared $(CFLAGS) $(INC) $(DBUS_CFLAGS) $<
