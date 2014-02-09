#*******************************************************************************
# Copyright (c) 2008 Red Hat Inc. and others.
# All rights reserved. This program and the accompanying materials
# are made available under the terms of the Eclipse Public License v1.0
# which accompanies this distribution, and is available at
# http://www.eclipse.org/legal/epl-v10.html
#
# Contributors:
#     Red Hat Inc. - initial API and implementation
#*******************************************************************************

# Use this makefile to build ppc 32-bit.

GNOME_KEYRING_PREFIX = gnome-keyring

GNOME_KEYRING_CFLAGS = `pkg-config --cflags gnome-keyring-1`
GNOME_KEYRING_LIBS = `pkg-config --libs gnome-keyring-1`

BUILD_DIR = .

CFLAGS = -shared -fPIC

all: libkeystoregnome.so install

install: libkeystoregnome.so
	if test $(BUILD_DIR) != '.'; then cp -u $< $(BUILD_DIR)/$<; fi
	
clean:
	rm libkeystoregnome.so keystoreLinuxNative.o
	
libkeystoregnome.so: keystoreLinuxNative.o
	$(CC) $(CFLAGS) -o $@ $< $(GNOME_KEYRING_LIBS)
	
keystoreLinuxNative.o: keystoreLinuxNative.c
	$(CC) -c -m32 -fPIC $(TESTFLAGS) $(GNOME_KEYRING_CFLAGS) $<
