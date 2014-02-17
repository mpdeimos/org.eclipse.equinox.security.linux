/*******************************************************************************
 * Copyright (c) 2014 Martin Poehlmann <http://mpdeimos.com>.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/

#include "secretServiceDBus.h"
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

void _error(_Env* env, const char* message)
{
#if TEST
	fprintf(stderr, message);
#else
	(*env->jni)->ExceptionClear(env->jni);
	(*env->jni)->ThrowNew(env->jni, (*env->jni)->FindClass(env->jni, "java/lang/SecurityException"), message);
#endif
}

int _assert(_Env* env, dbus_bool_t condition, const char* message)
{
	if (condition)
	{
		return TRUE;
	}
	
	char error [192];
	snprintf(error, 192, "Assertion failed: %s\n", message);
	_error(env, error);
	return FALSE;
}

int _assert_no_error(_Env* env, const char* message)
{
	if (!dbus_error_is_set(env->error))
	{
		return TRUE;
	}
	
	char error [192];
	snprintf(error, 192, "%s: %s: %s", message, env->error->name, env->error->message);
	_error(env, error);
	
	dbus_error_free(env->error);
	
	return FALSE;
}

int _init_env(_Env* env, JNIEnv* jni)
{
	env->jni = jni;
	
	// Initialize the DBus error
	env->error = (DBusError*) malloc(sizeof(DBusError));
	dbus_error_init(env->error);

	// Open DBus user session connection
	env->connection = dbus_bus_get(DBUS_BUS_SESSION, env->error);
	
	if (!_assert(env, env->connection != NULL, "Could not create DBus connection.")
		|| !_assert_no_error(env, "Failed to open connection to DBus."))
	{
		return FALSE;
	}
	
	// Ensure we do not tear down the VM on connection exit
	dbus_connection_set_exit_on_disconnect(env->connection, FALSE);
	
	return TRUE;
}

void _free_env(_Env* env)
{
	// Free the error memory
	free(env->error);

	// Free the DBus connection handle
	if (env->connection != NULL)
	{
		dbus_connection_unref(env->connection);
	}
}

void _dbus_util_dict_put(DBusMessageIter* dict, const char* key, const char* value)
{
	DBusMessageIter entry;
	dbus_message_iter_open_container(dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
	dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
	dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &value);
	dbus_message_iter_close_container(dict, &entry);
}

void _dbus_util_append_attributes(DBusMessageIter* iter, const char* service, const char* user, const char* type)
{
	DBusMessageIter dict;
	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, "{ss}", &dict);
	_dbus_util_dict_put(&dict, "service", service);
	_dbus_util_dict_put(&dict, "user", user);
	_dbus_util_dict_put(&dict, "type", type);
	dbus_message_iter_close_container(iter, &dict);
}

DBusMessage* _dbus_util_call(_Env* env, DBusMessage* message)
{
	DBusMessage* reply =  dbus_connection_send_with_reply_and_block(env->connection, message, -1, env->error);
	if (!_assert_no_error(env, "Error receiving message"))
	{
		dbus_message_unref(reply);
		return NULL;
	}
	return reply;
}

char* _dbus_secret_session_open(_Env* env)
{
	DBusMessage* message = dbus_message_new_method_call(
		"org.freedesktop.secrets",
		"/org/freedesktop/secrets",
		"org.freedesktop.Secret.Service",
		"OpenSession"
	);
	
	const char* algorithm = "plain";
	const char* nothing = "";
	DBusMessageIter args;
	DBusMessageIter input;
	dbus_message_iter_init_append(message, &args);
	dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &algorithm);
	dbus_message_iter_open_container(&args, DBUS_TYPE_VARIANT, "s", &input);
	dbus_message_iter_append_basic(&input, DBUS_TYPE_STRING, &nothing);
	dbus_message_iter_close_container(&args, &input);
	
	DBusMessage* reply = _dbus_util_call(env, message);
	if (reply == NULL)
	{
		dbus_message_unref(message);
		return NULL;
	}
	
	DBusMessageIter result;
	dbus_message_iter_init(reply, &result);
	if (!_assert(env, dbus_message_iter_next(&result), "Result does not contain session path.")
		|| !_assert(env, dbus_message_iter_get_arg_type(&result) == DBUS_TYPE_OBJECT_PATH, "Expected item of type path."))
	{
		dbus_message_unref(reply);
		dbus_message_unref(message);
		return NULL;
	}

	const char* path;
	dbus_message_iter_get_basic(&result, &path);

	char* pathCopy = strdup(path);
	
	dbus_message_unref(reply);
	dbus_message_unref(message);
	
	return pathCopy;
}

void _dbus_secret_session_close(_Env* env, const char* sessionPath)
{
	DBusMessage* message = dbus_message_new_method_call(
		"org.freedesktop.secrets",
		sessionPath,
		"org.freedesktop.Secret.Session",
		"Close"
	); 
	
	dbus_connection_send(env->connection, message, NULL);
	dbus_connection_flush(env->connection);
	
	dbus_message_unref(message);
}

char* _dbus_secret_search(_Env* env, const char* service, const char* user, const char* type)
{
	DBusMessage* message = dbus_message_new_method_call(
		"org.freedesktop.secrets",
		"/org/freedesktop/secrets/collection/login",
		"org.freedesktop.Secret.Collection",
		"SearchItems"
	); 
	
	DBusMessageIter args;
	dbus_message_iter_init_append(message, &args);
	_dbus_util_append_attributes(&args, service, user, type);
	
	DBusMessage* reply = _dbus_util_call(env, message);
	if (reply == NULL)
	{
		dbus_message_unref(message);
		return NULL;
	}
	
	DBusMessageIter result;
	dbus_message_iter_init(reply, &result);
	DBusMessageIter resultArray;
	dbus_message_iter_recurse(&result, &resultArray);
	if (dbus_message_iter_get_arg_type(&resultArray) == DBUS_TYPE_INVALID // This means that no secret is found
		|| !_assert(env, dbus_message_iter_get_arg_type(&resultArray) == DBUS_TYPE_OBJECT_PATH, "Expected item of type path."))
	{
		dbus_message_unref(reply);
		dbus_message_unref(message);
		return NULL;
	}
	const char* path;
	dbus_message_iter_get_basic(&resultArray, &path); // we are just interested in the first element

	char* pathCopy = strdup(path);
	
	dbus_message_unref(reply);
	dbus_message_unref(message);
	
	return pathCopy;
}

char* _dbus_secret_get(_Env* env, const char* sessionPath, const char* secretPath)
{
	DBusMessage* message = dbus_message_new_method_call(
		"org.freedesktop.secrets",
		secretPath,
		"org.freedesktop.Secret.Item",
		"GetSecret"
	); 
	
	dbus_message_append_args(
		message,
		DBUS_TYPE_OBJECT_PATH, &sessionPath,
		DBUS_TYPE_INVALID
	);
	
	DBusMessage* reply = _dbus_util_call(env, message);
	if (reply == NULL)
	{
		dbus_message_unref(message);
		return NULL;
	}
	
	DBusMessageIter result;
	dbus_message_iter_init(reply, &result);
	DBusMessageIter dataStruct;
	dbus_message_iter_recurse(&result, &dataStruct);
	if (!_assert( // forward to 3rd element
			env,
			dbus_message_iter_next(&dataStruct)
			&& dbus_message_iter_next(&dataStruct),
			"Result does not contain secret."
		)
		|| !_assert(
			env,
			dbus_message_iter_get_arg_type(&dataStruct) == DBUS_TYPE_ARRAY
			&& dbus_message_iter_get_element_type (&dataStruct) == DBUS_TYPE_BYTE,
			"Expected array of bytes."))
	{
		dbus_message_unref(reply);
		dbus_message_unref(message);
		return NULL;
	}
	
	DBusMessageIter secretArray;
	dbus_message_iter_recurse (&dataStruct, &secretArray);
	
	const void* secretData;
	int secretSize;
	dbus_message_iter_get_fixed_array(&secretArray, &secretData, &secretSize);

	char* secret = strndup(secretData, secretSize);
	
	dbus_message_unref(reply);
	dbus_message_unref(message);
	
	return secret;
}

char* _dbus_secret_create(_Env* env, const char* sessionPath, const char* service, const char* user, const char* type, const char* secret)
{
	DBusMessage* message = dbus_message_new_method_call(
		"org.freedesktop.secrets",
		"/org/freedesktop/secrets/collection/login",
		"org.freedesktop.Secret.Collection",
		"CreateItem"
	); 
	
	dbus_bool_t overwrite = TRUE;
	const char* propLabelName = "org.freedesktop.Secret.Item.Label";
	const char* propAttributesName = "org.freedesktop.Secret.Item.Attributes";
	const char* content_type = "text/plain";
	const char* empty = "";
	DBusMessageIter args;
	dbus_message_iter_init_append(message, &args);
	
	DBusMessageIter props;
	dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY, "{sv}", &props);
	
	  DBusMessageIter propLabel;
	  dbus_message_iter_open_container(&props, DBUS_TYPE_DICT_ENTRY, NULL, &propLabel);
	    dbus_message_iter_append_basic(&propLabel, DBUS_TYPE_STRING, &propLabelName);
	    
	    DBusMessageIter propLabelVariant;
	    dbus_message_iter_open_container(&propLabel, DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &propLabelVariant);
	      dbus_message_iter_append_basic(&propLabelVariant, DBUS_TYPE_STRING, &service);
	    dbus_message_iter_close_container(&propLabel, &propLabelVariant);
	  dbus_message_iter_close_container(&props, &propLabel);

	  DBusMessageIter propAttributes;
	  dbus_message_iter_open_container(&props, DBUS_TYPE_DICT_ENTRY, NULL, &propAttributes);
	    dbus_message_iter_append_basic(&propAttributes, DBUS_TYPE_STRING, &propAttributesName);
	    
	    DBusMessageIter propAttributesVariant;
	    dbus_message_iter_open_container(&propAttributes, DBUS_TYPE_VARIANT, "a{ss}", &propAttributesVariant);
	      _dbus_util_append_attributes(&propAttributesVariant, service, user, type);
	    dbus_message_iter_close_container(&propAttributes, &propAttributesVariant);
	  dbus_message_iter_close_container(&props, &propAttributes);
	
	dbus_message_iter_close_container(&args, &props);
	
	DBusMessageIter secretStruct;
	dbus_message_iter_open_container(&args, DBUS_TYPE_STRUCT, NULL, &secretStruct);
	  dbus_message_iter_append_basic(&secretStruct, DBUS_TYPE_OBJECT_PATH, &sessionPath);
	  
	  DBusMessageIter secretParam;
	  dbus_message_iter_open_container(&secretStruct, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE_AS_STRING, &secretParam);
	    dbus_message_iter_append_fixed_array(&secretParam, DBUS_TYPE_BYTE, &empty, 0);
	  dbus_message_iter_close_container(&secretStruct, &secretParam);
	  
	  DBusMessageIter secretData;
	  dbus_message_iter_open_container(&secretStruct, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE_AS_STRING, &secretData);
	    dbus_message_iter_append_fixed_array(&secretData, DBUS_TYPE_BYTE, &secret, strlen(secret));
	  dbus_message_iter_close_container(&secretStruct, &secretData);
	  
	  dbus_message_iter_append_basic(&secretStruct, DBUS_TYPE_STRING, &content_type);
	dbus_message_iter_close_container(&args, &secretStruct);
	
	dbus_message_iter_append_basic(&args, DBUS_TYPE_BOOLEAN, &overwrite);
	
	DBusMessage* reply = _dbus_util_call(env, message);
	if (reply == NULL)
	{
		dbus_message_unref(message);
		return NULL;
	}
	
	const char* secretPath;
	dbus_message_get_args(reply, env->error, 
		DBUS_TYPE_OBJECT_PATH, &secretPath,
		DBUS_TYPE_INVALID
	);
	if (!_assert_no_error(env, "Error reading message."))
	{
		dbus_message_unref(reply);
		dbus_message_unref(message);
		return NULL;
	}

	char* pathCopy = strdup(secretPath);
	
	dbus_message_unref(reply);
	dbus_message_unref(message);
	
	return pathCopy;
}
