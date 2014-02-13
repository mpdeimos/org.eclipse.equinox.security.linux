/*******************************************************************************
 * Copyright (c) 2014 Martin Poehlmann <http://mpdeimos.com>.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
#if TEST

#include "secretServiceDBus.prv.h"
#include "secretServiceDBus.h"
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

/** For testing via the cmd line */
void main(int argc, char** argv)
{
	// Initialize the DBus connection
	_Env env;
	if (!_init_env(&env, NULL))
	{
		printf("Environment not initialized. Exit.\n");
		return;
	}
	
	// Open a session for the secret service
	char* session = _dbus_secret_session_open(&env);
	if (session == NULL)
	{
		printf("Session not created. Exit.\n");
		return;
	}
	printf("session: %s\n", session);
	
	// Search for the stored secret
	char* path = _dbus_secret_search(&env, "org.eclipse.equinox.internal.security.linux", "mpdeimos", "master");
	if (path == NULL) // Secret not found
	{
		printf("path: %s\n", "NULL");
		
		// Create a new password
		path = _dbus_secret_create(&env, session, "org.eclipse.equinox.internal.security.linux", "mpdeimos", "master", "foo");
		if (path == NULL)
		{
			printf("Secret not created. Exit.\n");
			return;
		}
		printf("path: %s\n", path);
		
		// Search for the new password
		path = _dbus_secret_search(&env, "org.eclipse.equinox.internal.security.linux", "mpdeimos", "master");
		if (path == NULL)
		{
			printf("Secret not searchable. Exit.\n");
			return;
		}

	}
	printf("path: %s\n", path);
	
	// Get the actual password string
	char* secret = _dbus_secret_get(&env, session, path);
	if (secret == NULL)
	{
		printf("Secret not retrievable. Exit.\n");
		return;
	}
	printf("secret: %s\n", secret);
	
	// Close the secret service session
	_dbus_secret_session_close(&env, session);
	
	_free_env(&env);
	
	// Free allocated data
	free(secret);
	free(path);
	free(session);
}

#endif
