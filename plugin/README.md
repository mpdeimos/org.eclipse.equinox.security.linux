Eclipse Secure Storage for Linux
================================

Provides an Eclipse Secure Storage implementation for Linux via ~~[Java DBus](http://dbus.freedesktop.org/doc/dbus-java/)~~ [DBus Secret Service](http://standards.freedesktop.org/secret-service/).
The communication with DBus is written in C and has no other dependencies than `libdbus`

Notes
-----
* The implementation is merely experimental and not suited for daily use.
* This is a fork of the secure storage proposal offered in [this bug report](https://bugs.eclipse.org/bugs/show_bug.cgi?id=234509), but using `libdbus` directly instead of `libgnomekeyring`.
