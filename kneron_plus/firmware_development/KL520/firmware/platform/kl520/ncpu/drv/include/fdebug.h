#ifndef FDEBUG_H
#define FDEBUG_H

#include "kdrv_uart.h"

#define DO_ASSERT

#if defined(NO_PRINT)
	#define fLib_printf(args...)
	#define printk(args...)
	#define PUTCHAR(port_no,c)
	#define GETS				fLib_gets
	#define GETCHAR				fLib_getchar
	#define GETCHARTO			fLib_getchar_timeout
	#define GETCH				fLib_getch
#else	//NO_PRINT

#if !defined(WIN32)
	#define fLib_printf(args...)		fLib_printf(args)
	#define printk(args...)		fLib_printf(args)
	#define PUTCHAR(port_no,c)			fLib_putchar(port_no,c)
	#define GETS				fLib_gets				//pattern_gets
	#define GETCHAR				fLib_getchar
	#define GETCHARTO			fLib_getchar_timeout
	#define GETCH				fLib_getch
#endif

#endif //NO_PRINT



#if defined(RTL_SIM)
	#define finish(args...)		writel(0x01234567, AHBC_FTAHBC020S_PA_BASE+0x80000)
	#define pass(args...)		writel(0x01234568, AHBC_FTAHBC020S_PA_BASE+0x80000)
	#define fail(args...)		writel(0x01234569, AHBC_FTAHBC020S_PA_BASE+0x80000); \
								writel(0x01234567, AHBC_FTAHBC020S_PA_BASE+0x80000)
	#define warn(args...)		writel(0x01234570, AHBC_FTAHBC020S_PA_BASE+0x80000)
	#define debugport(code)		writel(code, AHBC_FTAHBC020S_PA_BASE+0x80000)

	#define ASSERT(x)   do { \
							if (!(x)) 	\
								for (;;)	\
									; 		\
						} while (0)

#else
	#if defined(DO_ASSERT)
		#define finish			fLib_printf
		#define pass			fLib_printf
		#define fail(args...)   do { \
									fLib_printf(args);				\
									fLib_printf(" failure at %s:%d/%s()!\n", __FILE__, __LINE__, __FUNCTION__);	\
									for (;;)	\
										;		\
								} while (0)
									
		#define warn			fLib_printf
		#define debugport(code)
		#define BUG()		do {	\
							fLib_printf("BUG: failure at %s:%d/%s()!\n", __FILE__, __LINE__, __FUNCTION__); \
							for (;;) ; \
						} while (0)

		#define WARN_ON(x)	ASSERT(!(x))
		#define BUG_ON(x)	ASSERT(!(x))
	
		#define ASSERT(x)   do { \
								if (!(x)) 		\
								{				\
									fLib_printf("assert failure at %s:%d/%s()!\n", __FILE__, __LINE__, __FUNCTION__);	\
									for (;;)	\
										;		\
								}				\
							} while (0)
							
		#define ASSERT_SKIP(x)

		#if !defined(WIN32)
			#define warn_on(exp, args...)  if (exp) { \
				fLib_printf("line %d, file \"%s\": ", __LINE__, __FILE__);  \
				fLib_printf(args); \
			}
		#endif //WIN32

	#else
		#define warn_on(exp, args...)
	#endif //DO_ASSERT
#endif




#endif //FDEBUG_H
