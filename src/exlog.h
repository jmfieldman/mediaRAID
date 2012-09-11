//
//  exlog.h
//  mediaRAID
//
//  Created by Jason Fieldman on 9/6/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#ifndef mediaRAID_exlog_h
#define mediaRAID_exlog_h


/* Global log file */
extern FILE *g_logfile;



typedef enum EXDebugComponent {
	
	/* General protocol */
	EXDBGCOMP_FUSE                       = 0x00000001,
	EXDBGCOMP_COMM                       = 0x00000002,
	EXDBGCOMP_VOLUME                     = 0x00000004,
	
	/* Performance */
	EXDBGCOMP_PERFORM                    = 0x40000000,
	
	/* Any */
	EXDBGCOMP_ANY                        = 0x7FFFFFFF,
	
} EXDebugComponent_t;

#define EXDBGCOMP_STR_FUSE               "FUSE"
#define EXDBGCOMP_STR_COMM               "COMM"
#define EXDBGCOMP_STR_VOLUME             "VOLUME"

#define EXDBGCOMP_STR_PERFORM            "PERFORM"
#define EXDBGCOMP_STR_ANY                "ANY"

typedef enum EXDebugLevel {
	
	EXDBGLVL_EMERG    = 0,
	EXDBGLVL_ALERT    = 1,
	EXDBGLVL_CRIT     = 2,
	EXDBGLVL_ERR      = 3,
	EXDBGLVL_WARN     = 4,
	EXDBGLVL_NOTICE   = 5,
	EXDBGLVL_INFO     = 6,
	EXDBGLVL_DBG      = 7,
	
} EXDebugLevel_t;

#define EXDBGLVL_STR_EMERG               "EMERG"
#define EXDBGLVL_STR_ALERT               "ALERT"
#define EXDBGLVL_STR_CRIT                "CRIT"
#define EXDBGLVL_STR_ERR                 "ERR"
#define EXDBGLVL_STR_WARN                "WARN"
#define EXDBGLVL_STR_NOTICE              "NOTICE"
#define EXDBGLVL_STR_INFO                "INFO"
#define EXDBGLVL_STR_DBG                 "DEBUG"



/* The global debug flags */
#ifndef EXDEBUGCOMPONENTS
#define EXDEBUGCOMPONENTS EXDBGCOMP_ANY
#endif

#ifndef EXDEBUGLEVEL
#define EXDEBUGLEVEL EXDBGLVL_DBG
#endif


#ifndef EXDEBUGENABLED
#define EXLog( _component, _level, _fmt, _fmtargs... )
#define EXDebuggingComponent( _component )             NO
#define EXDebuggingLevel( _level )                     NO
#define EXDebugging( _component , _level )             NO
#define Timing_MarkStartTime()
#define Timing_GetElapsedTime() (0.0)
#else


#define EXLog( _component, _level, _fmt, _fmtargs... )  do { \
if (!(EXDEBUGCOMPONENTS | EXDBGCOMP_##_component)) break;             \
if (!g_logfile) break;                                                \
if (EXDBGLVL_##_level > EXDEBUGLEVEL) break;                          \
char log_str[2048];                                                   \
/*char full_str[2048]; */                                                 \
snprintf(log_str, 2047, _fmt, ##_fmtargs );                           \
fprintf(g_logfile, "[ %9s : %6s : %3.3lf ] %s\n", EXDBGCOMP_STR_##_component , EXDBGLVL_STR_##_level , Timing_GetElapsedTime() ,  log_str);  \
/*snprintf(full_str, 2047, "[ %9s : %6s : %3.3lf ] %s\n", EXDBGCOMP_STR_##_component , EXDBGLVL_STR_##_level , Timing_GetElapsedTime() ,  log_str);  */ \
/*fprintf(g_logfile, "%s", full_str);                                            */           \
/*[[EXRemoteLog sharedInstance] logString:fullLog]; */                           \
fflush(g_logfile); \
} while (0)



#define EXDebuggingComponent  ( _c ) ( EXDEBUGCOMPONENTS | EXDBGCOMP_##_c )
#define EXDebuggingLevel      ( _l ) ( EXDBGLVL_##_l <= EXDEBUGLEVEL )
#define EXDebugging  ( _comp, _lvl ) ( EXDebuggingComponent( _comp ) && EXDebuggingLevel ( _lvl ) )

void Timing_MarkStartTime(void);
double Timing_GetElapsedTime(void);

#endif



#endif
