#ifndef UTILITY_H
#define UTILITY_H


#ifdef __cplusplus
extern "C" {
#endif


#include "base.h"

#define	CMDLEN			50
#define	MAXARGS			20
	
#define divRoundDown(n,s)   ((n) / (s))
#define divRoundUp(n,s)     ((n+s-1)/(s))

#define ARRAY_SIZE(x) 		(sizeof(x) / sizeof((x)[0]))

#define RoundUp(val, units) \
		((((unsigned long)(val) + ((units) - 1)) / (units)) * (units))
#define RoundDown(val, units) \
		(((unsigned long)(val)/(units))*(units))
		

#define REG32(adr)             *(volatile u32 *)(adr)
#define Min(a,b)  (((a) < (b)) ? (a) : (b))
#define Max(a,b)  (((a) > (b)) ? (a) : (b))

#ifndef	isblank
#define	isblank(ch)	(((ch) == ' ') || ((ch) == '\t'))
#endif

int substring(char **ptr, char *string, char *pattern);
unsigned int atonum(char *val_str);


struct burnin_cmd
{
	char    *string;					/* command name */	
	void    (*burnin_routine)();		/* implementing routine */
};

typedef struct cmd { //bessel:move from Drvftsdc021.h
	s8 *name;
	s8 *usage;
	//bessel:For IAR, it's a error "a value of type "Us16 (*)(Us32, s8 **)" cannot be used to initialize an entity of type "s32 (*)(s32, s8 **)"
	 u16(*func) (u32 argc, s8 ** argv);// s32(*func) (s32 argc, s8 ** argv);
} cmd_t;

extern int substring(char **ptr, char *string, char *pattern);
extern unsigned int atonum(char *val_str);
extern u32 get_dex(void);
extern void PrintWelcomeMsg(struct burnin_cmd * cmd, int col_width);
extern void ManualTesting(struct burnin_cmd * cmd, int col_width, int have_back);
extern void mem_dump(unsigned int addr, int size);
extern void DumpData(u8 *pp, u16 start_addr, u32 size);
extern u32 makeargs(s8 * cmd, s32 * argcptr, s8 *** argvptr);
extern s32 do_help(s32 argc, s8 ** argv,cmd_t *input_CmdTbl);
extern u32 ExecCmd(s8 * cmdline,cmd_t *input_CmdTbl);
extern unsigned int atonum(char *val_str);

#ifdef __cplusplus
}
#endif



#endif
