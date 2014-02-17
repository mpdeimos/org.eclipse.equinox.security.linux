/*******************************************************************************
 * Copyright (c) 2014 Martin Poehlmann <http://mpdeimos.com>.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/

#include "secretServiceDBus-jni.h"
#include "secretServiceDBus.h"
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

char* __get_master_password(_Env* env, const char* service, const char* user);
char* __get_master_password(_Env* env, const char* service, const char* user);

/** Gets the password with DBus Secret Service. */
JNIEXPORT jstring JNICALL Java_org_eclipse_equinox_internal_security_linux_LinuxProvider_getMasterPassword(JNIEnv* jni, jobject jThis, jstring jService, jstring jUser)
{
	const char* service = (*jni)->GetStringUTFChars(jni, jService, NULL);
	const char* user = (*jni)->GetStringUTFChars(jni, jUser, NULL);
	
	jstring jResult = (*jni)->NewStringUTF(jni, NULL);
	
	_Env env;
	if (_init_env(&env, jni))
	{
		char* password = __get_master_password(&env, service, user);
		if (password != NULL)
		{
			jResult = (*jni)->NewStringUTF(jni, password);
			free(password);
		}
	}
	
	(*jni)->ReleaseStringUTFChars(jni, jService, service);
	(*jni)->ReleaseStringUTFChars(jni, jUser, user);

	_free_env(&env);

	return jResult;
}

/** Sets the password with DBus Secret Service. */
JNIEXPORT void JNICALL Java_org_eclipse_equinox_internal_security_linux_LinuxProvider_setMasterPassword(JNIEnv* jni, jobject this, jstring jService, jstring jUser, jstring jPassword)
{
	const char *service = (*jni)->GetStringUTFChars(jni, jService, NULL);
	const char *user = (*jni)->GetStringUTFChars(jni, jUser, NULL);
	const char *password = (*jni)->GetStringUTFChars(jni, jPassword, NULL);

	_Env env;
	if (_init_env(&env, jni))
	{
		__set_master_password(&env, service, user, password);
	}
	
	(*jni)->ReleaseStringUTFChars(jni, jService, service);
	(*jni)->ReleaseStringUTFChars(jni, jUser, user);
	(*jni)->ReleaseStringUTFChars(jni, jPassword, password);
	
	_free_env(&env);
}

/** Returns the master password for the given service and user, NULL if no password is found. The returned string has to be freed. */
char* __get_master_password(_Env* env, const char* service, const char* user)
{
	// Open a session for the secret service
	char* session = _dbus_secret_session_open(env);
	if (session == NULL)
	{
		return FALSE;
	}
	
	// Search for the stored secret
	char* path = _dbus_secret_search(env, service, user, "master");
	if (path == NULL) // Secret not found
	{
		free(session);
		return NULL;
	}
	
	// Get the actual password string
	char* secret = _dbus_secret_get(env, session, path);
	free(path);
	free(session);
	
	if (secret == NULL)
	{
		return NULL;
	}
	
	return secret;
}

/** Stores the master password for the given service and user. Returns FALSE in case of error. */
int __set_master_password(_Env* env, const char* service, const char* user, const char* password)
{
	// Open a session for the secret service
	char* session = _dbus_secret_session_open(env);
	
	if (session == NULL)
	{
		return FALSE;
	}
	
	// Create new password
	char* path = _dbus_secret_create(env, session, service, user, "master", password);
	free(session);
	
	if (path == NULL)
	{
		return FALSE;
	}
	
	free(path);
	
	return TRUE;
}
