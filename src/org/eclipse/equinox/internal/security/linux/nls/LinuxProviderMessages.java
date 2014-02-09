/*******************************************************************************
 * Copyright (c) 2008 IBM Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 * 
 * Contributors:
 *     IBM Corporation - initial API and implementation
 *******************************************************************************/
package org.eclipse.equinox.internal.security.linux.nls;

import org.eclipse.osgi.util.NLS;

public class LinuxProviderMessages extends NLS {

	private static final String BUNDLE_NAME = "org.eclipse.equinox.internal.security.linux.nls.messages"; //$NON-NLS-1$

	public static String getPasswordError;
	public static String setPasswordError;
	public static String newPasswordGenerated;

	static {
		// load message values from bundle file
		reloadMessages();
	}

	public static void reloadMessages() {
		NLS.initializeMessages(BUNDLE_NAME, LinuxProviderMessages.class);
	}
}