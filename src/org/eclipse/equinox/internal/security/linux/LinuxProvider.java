/*******************************************************************************
 * Copyright (c) 2008 IBM Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 * 
 * Contributors:
 *     Red Hat Inc. - initial API and implementation
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

public class LinuxProvider extends PasswordProvider {

	/**
	 * The length of the randomly generated password in bytes
	 */
	private final static int PASSWORD_LENGTH = 64;

	/**
	 * Name of the entry for the Gnome keyring
	 */
	static final private String serviceName = "org.eclipse.equinox.internal.security.linux"; //$NON-NLS-1$

	private native String getMasterPassword(String service, String account) throws SecurityException;

	private native void setMasterPassword(String serviceName, String accountName, String password) throws SecurityException;

	static {
		System.loadLibrary("keystoregnome"); //$NON-NLS-1$
	}

	public PBEKeySpec getPassword(IPreferencesContainer container, int passwordType) {

		// get the name of the current user
		String accountName;
		BundleContext context = AuthPlugin.getDefault().getBundleContext();
		if (context != null)
			accountName = context.getProperty("user.name"); //$NON-NLS-1$
		else
			accountName = System.getProperty("user.name"); //$NON-NLS-1$
		if (accountName == null)
			return null;

		boolean newPassword = ((passwordType & CREATE_NEW_PASSWORD) != 0);
		boolean passwordChange = ((passwordType & PASSWORD_CHANGE) != 0);

		if (!newPassword && !passwordChange) {
			try {
				return new PBEKeySpec(getKeyringPassword(serviceName, accountName).toCharArray());
			} catch (SecurityException e) {
				AuthPlugin.getDefault().logError(LinuxProviderMessages.getPasswordError, e);
				return null;
			}
		}

		byte[] rawPassword = new byte[PASSWORD_LENGTH];
		SecureRandom random = new SecureRandom();
		random.setSeed(System.currentTimeMillis());
		random.nextBytes(rawPassword);
		String newPasswordString = Base64.encode(rawPassword);

		// checking again in the retrieval case to minimize possible collisions
		if (!newPassword && !passwordChange) {
			try {
				return new PBEKeySpec(getKeyringPassword(serviceName, accountName).toCharArray());
			} catch (SecurityException e) {
				// ignore - we have already logged it above
			}
		}

		// add info message in the log
		AuthPlugin.getDefault().logMessage(LinuxProviderMessages.newPasswordGenerated);

		try {
			setKeyringPassword(serviceName, accountName, newPasswordString);
			return new PBEKeySpec(newPasswordString.toCharArray());
		} catch (SecurityException e) {
			AuthPlugin.getDefault().logError(LinuxProviderMessages.setPasswordError, e);
			return null;
		}
	}
}
