DBUS_PREFIX = dbus

DBUS_CFLAGS = `pkg-config --cflags dbus-1`
DBUS_LIBS = `pkg-config --libs dbus-1`

INC = -I/opt/java/include -I/opt/java/include/linux

BUILD_DIR = .

CFLAGS = -shared -fPIC

all: libSecretServiceDBus.so install

install: libSecretServiceDBus.so
	if test $(BUILD_DIR) != '.'; then cp -u $< $(BUILD_DIR)/$<; fi
	
clean:
	rm libSecretServiceDBus.so secretServiceDBus.o

secretServiceDBus: secretServiceDBus.o
	$(CC) -fPIC -o $@ $< $(DBUS_LIBS)

libSecretServiceDBus.so: secretServiceDBus.o
	$(CC) $(CFLAGS) -o $@ $< $(DBUS_LIBS)
	
secretServiceDBus.o: secretServiceDBus.c
	$(CC) -c -fPIC $(TESTFLAGS) $(INC) $(DBUS_CFLAGS) $<
