#include <stdio.h>
#include <glob.h>
#include <pthread.h>

#include "plugins.h"
#include "global_plugins.h"

#include "php.h"
#include "php_version.h"
#include "php_globals.h"
#include "php_variables.h"
#include "zend_modules.h"


#include "SAPI.h"

#include "php.h"

#include "zend.h"
#include "zend_extensions.h"

#include "php_ini.h"
#include "php_globals.h"
#include "php_main.h"
#include "fopen_wrappers.h"
#include "ext/standard/php_standard.h"
#include "ext/standard/info.h"

#include "zend_API.h"



#define MODULE_NAME "PHP" // Unique identifier


static int ape_cli_startup(sapi_module_struct *sapi_module) /* {{{ */
{
	if (php_module_startup(sapi_module, NULL, 0) == FAILURE) {
		return FAILURE;
	}
	php_printf("[Module] [%s] PHP %s Starting up...\n", MODULE_NAME, PHP_VERSION);
	return SUCCESS;
}
static int ape_cli_shutdown(sapi_module_struct *sapi_module) /* {{{ */
{
	return SUCCESS;
}

static int ape_cli_send_headers(sapi_headers_struct *sapi_headers TSRMLS_DC)
{
	return SAPI_HEADER_SENT_SUCCESSFULLY;
}
static int ape_cli_ub_write(const char *str, uint str_length TSRMLS_DC)
{
	printf("%s", str);

	return str_length;
}

static sapi_module_struct ape_sapi_module = 
{
	"APEHandler",                   /* name */
	"APE PHP Handler",          	/* pretty name */
									

	ape_cli_startup,            	 /* startup */
	ape_cli_shutdown,	 	/* shutdown */

	NULL,				/* activate */
	NULL,				/* deactivate */

	ape_cli_ub_write,           	/* unbuffered write */
	NULL,                 		/* flush */
	NULL,                         	/* get uid */
	NULL,                         	/* getenv */

	NULL,          			/* error handler */

	NULL,                         	/* header handler */
	ape_cli_send_headers,      	/* send headers handler */
	NULL,       			/* send header handler */

	NULL,                		/* read POST data */
	NULL,             		/* read Cookies */
	NULL,

	NULL,				/* register server variables */
	NULL,              		/* Log message */

	NULL,				/* Block interruptions */
	NULL,				/* Unblock interruptions */

	STANDARD_SAPI_MODULE_PROPERTIES
};





/* You must name this "infos_module" because of macro READ_CONF */
static ace_plugin_infos infos_module = {
	"PHP embed", 		// Module Name
	"0.01",			// Module Version
	"Anthony Catel",	// Module Author
	NULL	// Config file (from ./bin/) (can be NULL)
};

/* From facebook library */
#if 0
static int eval_string(const char *fmt, ...)
{
	char *data = NULL;
	int status;
	va_list ap;

	va_start(ap, fmt);


	zend_first_try {
		vspprintf(&data, 0, fmt, ap);
		status = zend_eval_string(data, NULL, "" TSRMLS_CC);
	} zend_catch {
		status = 0;
	} zend_end_try();

	if (data) {
		efree(data);
	}

	va_end(ap);
	return status;
}
#endif
static void ape_exec_script(char *file)
{	
	zend_file_handle file_handle;
	
	SG(request_info).request_method = NULL;
	SG(request_info).query_string = NULL;
	SG(request_info).content_type = NULL;
	SG(request_info).request_uri = file;
	SG(request_info).path_translated = file;
	SG(request_info).content_length = 0;
	SG(sapi_headers).http_response_code = 200;
	
	SG(server_context) = NULL;	

	if (php_request_startup(TSRMLS_C) == FAILURE) {
		printf("Cannot execute PHP...\n");
		return;
	}
	//CG(interactive) = 1;
	file_handle.handle.fp = VCWD_FOPEN(file,"rb");
	file_handle.filename = file;
	file_handle.type = ZEND_HANDLE_FP;
	file_handle.free_filename = 0;
	file_handle.opened_path = NULL;
	php_execute_script(&file_handle);
	
	
}

static void ape_call_method(zend_class_entry *ce, char *method_name TSRMLS_DC)
{
	zval *method, *dummy = NULL;
	MAKE_STD_ZVAL(method);
	ZVAL_STRING(method, method_name, 0);
	
	call_user_function_ex(&ce->function_table, NULL, method, &dummy, 0, NULL, 0, NULL TSRMLS_CC);
}

static void ape_init_class(char *classname TSRMLS_DC)
{
	zval *rv;
	zend_class_entry **ce;

	ALLOC_ZVAL(rv);
	Z_TYPE_P(rv) = IS_OBJECT;

	if (zend_lookup_class(classname, strlen(classname), &ce TSRMLS_CC) == SUCCESS) {
		if ((*ce)->constructor) {

			zend_fcall_info fci;
			zend_fcall_info_cache fcc;

			zval *retval_ptr = NULL;

			object_init_ex(rv, *ce);
		
			fci.size = sizeof(fci);
			fci.function_table = EG(function_table);
			fci.function_name = NULL;
			fci.symbol_table = NULL;
			fci.object_ptr = rv;
			fci.retval_ptr_ptr = &retval_ptr;
			fci.param_count = 0;
			fci.params = NULL;
			fci.no_separation = 1;
		
			fcc.initialized = 1;
			fcc.function_handler = (*ce)->constructor;
			fcc.calling_scope = EG(scope);
			fcc.called_scope = Z_OBJCE_P(rv);
			fcc.object_ptr = rv;
		
			zend_call_function(&fci, &fcc TSRMLS_CC);
			if (retval_ptr) {
				zval_ptr_dtor(&retval_ptr);
			}
		} else {
			object_init_ex(rv, *ce);
		}

		//ape_call_method(*ce, "__construct" TSRMLS_CC);
	}
}
static void *launch_php_script(void *params)
{
	char *input = (char *)params;
	char classname[33];
	if (strlen(input) <= 32) {
		ape_exec_script(input);
		input = strrchr(input, '/');
		*strchr(input, '.') = '\0';
		sprintf(classname, "ape_%s", &input[1]);

		ape_init_class(classname TSRMLS_CC);
	}
	
	free(params);
	return NULL;
}

static void init_module(acetables *g_ape) // Called when module is loaded (passed to APE_INIT_PLUGIN)
{
	int i;
	glob_t globbuf;
	pthread_t threads[32];
	
	sapi_startup(&ape_sapi_module);
	ape_sapi_module.startup(&ape_sapi_module);	
	
	glob("./scripts/*.php", 0, NULL, &globbuf);
	for (i = 0; i < globbuf.gl_pathc; i++) {
		
		pthread_create(&threads[i], NULL, launch_php_script, (void *)xstrdup(globbuf.gl_pathv[i]));
	}
	globfree(&globbuf);
}
static USERS *ape_adduser(unsigned int fdclient, char *host, acetables *ace_tables)
{
	USERS *n;
	
	//call_user_function_ex(CG(function_table), &function_name, NULL, &rv, 0, NULL, 0, NULL TSRMLS_CC);	
	n = adduser(fdclient, host, ace_tables);

	

	return n;	
}

/* See plugins.h for prototype */
static ace_callbacks callbacks = {
	ape_adduser,			/* Called when new user is added */
	NULL,				/* Called when a user is disconnected */
	NULL,				/* Called when new chan is created */
	NULL,				/* Called when a user join a channel */
	NULL				/* Called when a user leave a channel */
};

/* Registering module (arg1 : unique identifier, arg2 : init function, arg3 : Callbacks list) */
APE_INIT_PLUGIN(MODULE_NAME, init_module, callbacks)
