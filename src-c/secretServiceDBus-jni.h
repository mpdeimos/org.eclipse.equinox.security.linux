/*******************************************************************************
 * Copyright (c) 2014 Martin Poehlmann <http://mpdeimos.com>.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
 
#include <jni.h>

#ifndef __SECRET_SERVICE_DBUS__JNI__H
#define __SECRET_SERVICE_DBUS__JNI__H

/*
 * Class:     LinuxProvider
 * Method:    getKeyringPassword
 * Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_eclipse_equinox_internal_security_linux_LinuxProvider_getKeyringPassword
  (JNIEnv *, jobject, jstring, jstring);

/*
 * Class:     LinuxProvider
 * Method:    setKeyringPassword
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_org_eclipse_equinox_internal_security_linux_LinuxProvider_setKeyringPassword
  (JNIEnv *, jobject, jstring, jstring, jstring);

#endif
