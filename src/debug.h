#ifndef MY_DEBUG_H
#define MY_DEBUG_H


#ifdef MG_DEBUG

	#include <stdio.h>
	
	#define FUN_START printf( \
			 "{{{----[%s]---<file: %s>--<line:%d>----{{{\n", \
			 __FUNCTION__, __FILE__, __LINE__);

	#define FUN_END printf( \
			 "}}}----[%s]---<file: %s>--<line:%d>----}}}\n\n", \
			 __FUNCTION__, __FILE__, __LINE__);
	
	#define TEST_VAL(val, FORMAT) \
			printf("[%s]=<"#FORMAT">\n", #val, (val))
	
	#define TEST_INFO(str) \
			printf("%s\n", (str))

	#define TEST_SENTENCE(sent) \
			printf("%s\n", #sent); \
			sent

	
#else
	#define FUN_START	(void)0;
	#define FUN_END		(void)0;
	
	#define TEST_VAL(val, FORMAT)	(void)0
	#define TEST_INFO(str)			(void)0
	
	#define TEST_SENTENCE(sent)	sent

#endif

#endif
