/*******************************************************************************
 * Copyright (c) 2008 Red Hat Inc. and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 * 
 * Contributors:
 *     Red Hat Inc. - initial API and implementation
 *******************************************************************************/

#include <jni.h>
#include "keystoreLinuxNative.h"
#include <gnome-keyring.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

/**
 * Implements the get password functionality.
 */ 
JNIEXPORT jstring JNICALL Java_org_eclipse_equinox_internal_security_linux_LinuxProvider_getKeyringPassword(JNIEnv *env, jobject this, jstring serviceName, jstring accountName) {
	GnomeKeyringResult status;
	const char *serviceNameUTF = (*env)->GetStringUTFChars(env, serviceName, NULL);
	const char *accountNameUTF = (*env)->GetStringUTFChars(env, accountName, NULL);
	GList *pwlist = NULL;
	jstring result;
	GnomeKeyringInfo *keyringinfo = NULL;
	int defaultKeyringPasswordFound = 0;

	status = gnome_keyring_get_info_sync("default", &keyringinfo);
	
	if (status == GNOME_KEYRING_RESULT_NO_SUCH_KEYRING) {
		(*env)->ExceptionClear(env);
		char buffer [60];
		sprintf(buffer, "Could not find password in default keyring.  Result: %d", (int) status);
		(*env)->ThrowNew(env, (* env)->FindClass(env, "java/lang/SecurityException"), buffer);
	} else if (status == GNOME_KEYRING_RESULT_OK) {
		gboolean isLocked = gnome_keyring_info_get_is_locked(keyringinfo);
		gnome_keyring_info_free(keyringinfo);
		if (isLocked) {
			status = gnome_keyring_unlock_sync("default", NULL);
			if (status != GNOME_KEYRING_RESULT_OK) {
				(*env)->ExceptionClear(env);
				char buffer [60];
				sprintf(buffer, "Could not unlock default keyring.  Result: %d", (int) status);
				(*env)->ThrowNew(env, (* env)->FindClass(env, "java/lang/SecurityException"), buffer);
			}
		}
	}

	// We expect to find the password in the equinox keyring.
	status = gnome_keyring_find_network_password_sync (
			accountNameUTF, /* user */
			NULL, /* domain */
			NULL, /* server */
			serviceNameUTF, /* object */
			NULL, /* protocol */
			NULL, /* authtype */
			0, /* port */
			&pwlist
	);
	
	// free the UTF strings
	(*env)->ReleaseStringUTFChars( env, serviceName, serviceNameUTF );
	(*env)->ReleaseStringUTFChars( env, accountName, accountNameUTF );

	// throw an exception if we have an error
	if (status != GNOME_KEYRING_RESULT_OK) {
		(*env)->ExceptionClear(env);
		char buffer [60];
		sprintf(buffer, "Could not obtain password.  Result: %d", (int) status);
		(*env)->ThrowNew(env, (* env)->FindClass(env, "java/lang/SecurityException"), buffer);
	}

	if (pwlist != NULL) {
		GnomeKeyringNetworkPasswordData *ptr = (GnomeKeyringNetworkPasswordData *)(pwlist->data);
		/* if (!strcmp(ptr->keyring, "equinox")) { */
			// massage the string into a Java-friendly UTF-8 string
			char *truncatedPassword = (char *) malloc(strlen(ptr->password) * sizeof(char) + 1);
			strcpy(truncatedPassword, ptr->password);
			result = (*env)->NewStringUTF(env, truncatedPassword);
			defaultKeyringPasswordFound = 1;
			free(truncatedPassword);
	}
	
	gnome_keyring_network_password_list_free(pwlist);
	
	// throw an exception if we couldn't find the password in the default keyring
	if (!defaultKeyringPasswordFound) {
		(*env)->ExceptionClear(env);
		char buffer [] = "Could not obtain password from default keyring.";
		(*env)->ThrowNew(env, (* env)->FindClass(env, "java/lang/SecurityException"), buffer);
	}

	return result;
}

/**
 * Implements the set password functionality.
 */ 
JNIEXPORT void JNICALL Java_org_eclipse_equinox_internal_security_linux_LinuxProvider_setKeyringPassword(JNIEnv *env, jobject this, jstring serviceName, jstring accountName, jstring password) {
	GnomeKeyringResult status;
	const char *serviceNameUTF = (*env)->GetStringUTFChars(env, serviceName, NULL);
	const char *accountNameUTF = (*env)->GetStringUTFChars(env, accountName, NULL);
	const char *passwordUTF = (*env)->GetStringUTFChars(env, password, NULL);
	GnomeKeyringInfo *keyringinfo = NULL;
	guint32 id = 0;
	
	// attempt to add the password as a network password, using the
	// service as a remote object and specifying the keyring as "equinox".
	status = gnome_keyring_set_network_password_sync ("default", 
			accountNameUTF, /* user */
			NULL, /* domain */
			NULL, /* server */
			serviceNameUTF, /* object */
			NULL, /* protocol */
			NULL, /* authtype */
			0, /* port */
			passwordUTF,
			&id);

	// free the UTF strings
	(*env)->ReleaseStringUTFChars( env, serviceName, serviceNameUTF );
	(*env)->ReleaseStringUTFChars( env, accountName, accountNameUTF );
	(*env)->ReleaseStringUTFChars( env, password, passwordUTF );

	// throw an exception if it didnt work
	if (status != GNOME_KEYRING_RESULT_OK) {
		(*env)->ExceptionClear(env);
		char buffer [60];
		sprintf(buffer, "Could not set password.  Result: %d", (int) status);
		(*env)->ThrowNew(env, (* env)->FindClass(env, "java/lang/SecurityException"), buffer);
	}
}

