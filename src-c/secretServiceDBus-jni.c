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

/** Gets the password with DBus Secret Service. */
JNIEXPORT jstring JNICALL Java_org_eclipse_equinox_internal_security_linux_LinuxProvider_getMasterPassword(JNIEnv* env, jobject this, jstring serviceName, jstring accountName) {
	const char *serviceNameUTF = (*env)->GetStringUTFChars(env, serviceName, NULL);
	const char *accountNameUTF = (*env)->GetStringUTFChars(env, accountName, NULL);
	jstring result;

	// free the UTF strings
	(*env)->ReleaseStringUTFChars( env, serviceName, serviceNameUTF );
	(*env)->ReleaseStringUTFChars( env, accountName, accountNameUTF );

	return result;
}

/** Sets the password with DBus Secret Service. */
JNIEXPORT void JNICALL Java_org_eclipse_equinox_internal_security_linux_LinuxProvider_setMasterPassword(JNIEnv* env, jobject this, jstring serviceName, jstring accountName, jstring password) {
	const char *serviceNameUTF = (*env)->GetStringUTFChars(env, serviceName, NULL);
	const char *accountNameUTF = (*env)->GetStringUTFChars(env, accountName, NULL);
	const char *passwordUTF = (*env)->GetStringUTFChars(env, password, NULL);

	// free the UTF strings
	(*env)->ReleaseStringUTFChars( env, serviceName, serviceNameUTF );
	(*env)->ReleaseStringUTFChars( env, accountName, accountNameUTF );
	(*env)->ReleaseStringUTFChars( env, password, passwordUTF );
}
