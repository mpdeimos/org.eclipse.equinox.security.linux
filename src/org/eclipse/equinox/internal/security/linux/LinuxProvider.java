/*******************************************************************************
 * Copyright (c) 2014 Martin Poehlmann <http://mpdeimos.com>.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.eclipse.equinox.internal.security.linux;

import java.security.SecureRandom;

import javax.crypto.spec.PBEKeySpec;

import org.eclipse.equinox.internal.security.auth.AuthPlugin;
import org.eclipse.equinox.internal.security.linux.nls.LinuxProviderMessages;
import org.eclipse.equinox.internal.security.storage.Base64;
import org.eclipse.equinox.security.storage.provider.IPreferencesContainer;
import org.eclipse.equinox.security.storage.provider.PasswordProvider;
import org.osgi.framework.BundleContext;

/**
 * Provides password for Linux platforms using DBus Secret Service.
 */
public class LinuxProvider extends PasswordProvider {
	/**
	 * Name of the password in the DBus Secret Service.
	 */
	static final private String SERVICE_ID = "org.eclipse.equinox.security.linux"; //$NON-NLS-1$

	/**
	 * The length of the randomly generated password in bytes.
	 */
	private final static int PASSWORD_LENGTH = 64;

	/**
	 * Uses JNI to retrieve the password for the given user from DBus Secret Service.
	 */
	private native String getMasterPassword(String service, String user) throws SecurityException;

	/**
	 * Uses JNI to store the password for the given user in DBus Secret Service.
	 */
	private native void setMasterPassword(String service, String user, String password) throws SecurityException;

	static {
		System.loadLibrary("secretServiceDBus"); //$NON-NLS-1$
	}

	/**
	 * {@inheritDoc}
	 */
	public PBEKeySpec getPassword(IPreferencesContainer container, int passwordType) {

		// get the name of the current user
		String user;
		BundleContext context = AuthPlugin.getDefault().getBundleContext();
		if (context != null)
			user = context.getProperty("user.name"); //$NON-NLS-1$
		else
			user = System.getProperty("user.name"); //$NON-NLS-1$
		if (user == null)
			return null;

		if ((passwordType & CREATE_NEW_PASSWORD) != 0 || (passwordType & PASSWORD_CHANGE) != 0) {
			return createAndStorePassword(user);
		}

		try {
			String masterPassword = getMasterPassword(SERVICE_ID, user);
			return new PBEKeySpec(masterPassword.toCharArray());
		} catch (SecurityException e) {
			AuthPlugin.getDefault().logError(LinuxProviderMessages.getPasswordError, e);
			return null;
		}
	}

	/** Creates and stores the password in the DBus Secret Storage and returns the created password. */
	private PBEKeySpec createAndStorePassword(String user) {
		byte[] rawPassword = new byte[PASSWORD_LENGTH];
		SecureRandom random = new SecureRandom();
		random.setSeed(System.currentTimeMillis());
		random.nextBytes(rawPassword);
		String masterPassword = Base64.encode(rawPassword);
		AuthPlugin.getDefault().logMessage(LinuxProviderMessages.newPasswordGenerated);
		try {
			setMasterPassword(SERVICE_ID, user, masterPassword);
		} catch (SecurityException e) {
			AuthPlugin.getDefault().logError(LinuxProviderMessages.setPasswordError, e);
			return null;
		}
		return new PBEKeySpec(masterPassword.toCharArray());
	}
}
