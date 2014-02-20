Eclipse Secure Storage for Linux
================================

Provides an Eclipse Secure Storage implementation for Linux via [DBus Secret Service](http://standards.freedesktop.org/secret-service/).
The communication with DBus is written in C and has no other dependencies than `libdbus`.

Installation
------------
* Add my [Eclipse Update Site](http://mpdeimos.com/eclipse-updatesite/) and install the feature from there.
* Remove existing passwords from Eclipse Secure Storage so they can be re-added and secured with DBus Secret Service.

Limitations
-----------
* The plugin cannot handle locked keyrings yet (this is the case for users with enabled auto-login).
* The implementation is not tested on each and every Linux distribution. However, it seems works fine on 64 Bit Arch Linux and 32 Bit Ubuntu 13.10.

History
-------
* I've initially forked the Linux Secure Storage proposal offered in [this bug report](https://bugs.eclipse.org/bugs/show_bug.cgi?id=234509), but wanted to re-write it using [Java DBus](http://dbus.freedesktop.org/doc/dbus-java/). As Java DBus requires several Java and native libraries I've decided against.
* However, the `libgnomekeyring` binding has been replaced by a binding to `libdbus`, so I would no longer consider this a fork of the above mentioned implementation.
