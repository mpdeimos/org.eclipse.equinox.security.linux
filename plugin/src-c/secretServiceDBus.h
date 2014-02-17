/*******************************************************************************
 * Copyright (c) 2014 Martin Poehlmann <http://mpdeimos.com>.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/

#include <jni.h>
#include <dbus/dbus.h>

#ifndef __SECRET_SERVICE_DBUS__H
#define __SECRET_SERVICE_DBUS__H

/** Envoronment containing a JNI reference, the DBus connection and the DBus error struct. */
typedef struct {
	JNIEnv* jni;
	DBusConnection* connection;
	DBusError* error;
} _Env;

/** Asserts that the condition holds, notifies about the error, and returns FALSE in case of an error. */
int   _assert(_Env* env, dbus_bool_t condition, const char* message);

/** Asserts that no DBus error occured, notifies about the error, and returns FALSE in case of an error. */
int   _assert_no_error(_Env* env, const char* message);

/** Initializes the DBus and JNI environment and returns FALSE in case of an error.  */
int _init_env(_Env* env, JNIEnv* jni);

/** Frees environment handles and memory. */
void _free_env(_Env* env);

/** Opens a connection to the secret service and returns the session path. Returns NULL in case of error. */
char* _dbus_secret_session_open(_Env* env);

/** Closes the connection to the secret service. */
void  _dbus_secret_session_close(_Env* env, const char* sessionPath);

/** Searches the secret service for a path to a single secret according to the specified service, user and type name. Returns NULL in case no secret could be found. */
char* _dbus_secret_search(_Env* env, const char* service, const char* user, const char* type);

/** Returns the secret stored under the given path. Returns NULL in case of error.*/
char* _dbus_secret_get(_Env* env, const  char* sessionPath, const  char* secretPath);

/** Creates a new secret for the specified service, user and type name and returns its path. Returns NULL in case of error. */
char* _dbus_secret_create(_Env* env, const  char* sessionPath, const char* service, const char* user, const char* type, const char* secret);

#endif
