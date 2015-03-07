#ifndef __LOGGING_H__
	#define __LOGGING_H__

	#ifndef NETWORK_LOGGING
	   #define NETWORK_LOGGING 0
	#endif
	#if(NETWORK_LOGGING != 2 && NETWORK_LOGGING != 1) 
	    #undef NETWORK_LOGGING
	    #define NETWORK_LOGGING 0
	#endif
	#define NETWORK_LOG_LEVEL_2_NUM(s,d) if(NETWORK_LOGGING == 2) fprintf(stderr, "%ld:%s %d\n",syscall(SYS_gettid) , (s), (d) );
	#define NETWORK_LOG_LEVEL_2(s) if(NETWORK_LOGGING == 2) fprintf(stderr, "%ld:%s\n", syscall(SYS_gettid),  (s) );
	#define NETWORK_LOG_LEVEL_1(s) if(NETWORK_LOGGING >= 1) fprintf(stderr, "%ld:%s\n", syscall(SYS_gettid),  (s) );

	#ifndef DATABASE_LOGGING
		#define DATABASE_LOGGING 0
	#endif
	#if(DATABASE_LOGGING != 1)
		#ifdef DATABASE_LOGGING
			#undef DATABASE_LOGGING
		#endif
		#define DATABASE_LOGGING 0
	#endif
	#define LOGDB if(DATABASE_LOGGING == 1) fprintf(stderr, "%ld:DB: %s\n", syscall(SYS_gettid), query);
	#define LOGDBTRANS(status) if(DATABASE_LOGGING == 1) fprintf(stderr, "%ld:Transaction has been %s\n", syscall(SYS_gettid) ,status);
	
	#ifndef BOOT_LOGGING
    	#define BOOT_LOGGING 0
	#endif
	#if(BOOT_LOGGING != 1)
	    #ifdef BOOT_LOGGING
	        #undef BOOT_LOGGING
	    #endif
	    #define BOOT_LOGGING 0
	#endif
	/*pre must be a string declared like "sting" not a variable*/
	#define BOOT_LOG_STR(pre,s) if(BOOT_LOGGING == 1) fprintf(stderr, pre "%ld:%s\n", syscall(SYS_gettid) , (s));
	#define BOOT_LOG_NUM(pre,d) if(BOOT_LOGGING == 1) fprintf(stderr, pre "%d\n", (d));


#endif