DBUS_CFLAGS = `pkg-config --cflags dbus-1`
DBUS_LIBS = `pkg-config --libs dbus-1`

INC = -I/opt/java/include -I/opt/java/include/linux

CFLAGS = -fPIC

test: CFLAGS += -DTEST

all: libSecretServiceDBus.so

clean:
	rm -f *.so *.o "test"

test: secretServiceDBus-test.o secretServiceDBus.o
	$(CC) -fPIC -o $@ $^ $(DBUS_LIBS)
	./test

libSecretServiceDBus.so: secretServiceDBus-jni.o secretServiceDBus.o
	$(CC) -shared $(CFLAGS) -o $@ $< $(DBUS_LIBS)

%.o: %.c
	$(CC) -c $(CFLAGS) $(INC) $(DBUS_CFLAGS) $<
