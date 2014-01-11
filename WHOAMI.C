/*
 * whoami (vio and pm version.) for OS/2 internet access. v1.2
 *
 * Simply tells you what hostname and IP address is assigned to your station.
 *
 * Yes, this is partly 'sloppy' coding. It works anyway. :) Actually, I was
 *   experimenting with having the source code for the vio (text-based) and
 *   presentation manager versions of this program in the same file.
 *                             ***BIG MISTAKE***
 *    The product of this experiment was a whole bunch of #ifdefs. Yick.
 *
 * This file looks best if tab stops are set to 4 characters.
 *
 * To compile a Presentation Manager application, #define PM_APP somewhere,
 *  otherwise #define VIO_APP or leave it alone (VIO_APP will be #defined by
 *  default.)
 *
 * This compiles for me under EMX 0.9b with no complaint.
 *   (even with -Wall! Goddamn I'm proud!  :)   )
 *
 * Feel free to use this code in anyway you see fit.
 *  Originally written by Ryan C. Gordon, 1996.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>

#ifdef PM_APP
	#define INCL_WIN
	#define INCL_GPI
	#include <os2.h>
	#ifdef VIO_APP
		#undef VIO_APP
	#endif

	#define UM_WHOAMI_DONE	WM_USER + 2
	#define ADDSPACE		6

#else
	#if !defined VIO_APP
		#define VIO_APP
	#endif

		/* we don't need to #include <os2.h>,	 */
		/*	so here's some boolean data types... */
	typedef unsigned long BOOL;
	#define TRUE	(1)
	#define FALSE	(0)
#endif

	/* constants : show IP address, hostname, or both? */
#define JUSTSHOW_BOTH 0
#define JUSTSHOW_IP   1
#define JUSTSHOW_HOST 2

	/* command line arguments. */
static char ARG_KEEPON[]   = "/k";		/* keep trying until info is found. */
static char ARG_JUSTIP[]   = "/ip";		/* only display the IP address.		*/
static char ARG_JUSTHOST[] = "/host";	/* only display the hostname.		*/
static char ARG_EXE[]	   = "/exe";	/* execute w/ env variable set.		*/

	/* string literals.  For translating to other languages... */
#ifdef PM_APP
 static char TITLEBAR[]  = "PMwhoami v1.2";
 static char NO_PM_EXE[] = "Please use the text version of this program.";
#else
 static char ENVSTRING[] = "HOSTNAME";
#endif
 static char NOHOST[]    = "Couldn't get hostname information.";
 static char BOGUSARGS[] = "Invalid arguments.";


	/* global variables. */
BOOL bogusargs = FALSE;
BOOL keepon = FALSE;
BOOL shouldexe = FALSE;
int justshow = JUSTSHOW_BOTH;
int errorlevel = 0;
char myname[500];
char *iakenv;


#ifdef PM_APP
	HWND hwndFrame, hwndClient;
#endif

void getinfo(void)
{
	struct hostent *hp;
	BOOL getout = FALSE;
	unsigned int work;
	int looper;	
	char ipaddr[20];

#ifdef PM_APP
	if (shouldexe)
	{
		strcpy(myname, NO_PM_EXE);
		return;
	} /* if */
#endif

	if (bogusargs)
	{
		strcpy(myname, BOGUSARGS);
		return;
	} /* if */

	ipaddr[0] = '\0';

	while (!getout)
	{

		gethostname(myname, 256);
		hp = gethostbyname(myname);

		if (hp != NULL)
		{
			getout = TRUE;
			strcpy(myname, hp->h_name);

			if (justshow != JUSTSHOW_HOST)
			{
				strcat(myname, "  (");

				for (looper = 0; looper < hp->h_length; looper++)
				{
					work = (unsigned char) hp->h_addr_list[0][looper];
					_itoa(work, &ipaddr[strlen(ipaddr)], 10);
					strcat(ipaddr, ".");
				} /* for */

				ipaddr[strlen(ipaddr) - 1] = '\0';  /* lose last '.'... */

				if (justshow == JUSTSHOW_IP)
					strcpy(myname, ipaddr);
				else
				{
					strcat(myname, ipaddr);
					strcat(myname, ")");
				} /* else */

			} /* if */	

		} /* if */

		else if (keepon == FALSE)
		{
			strcpy(myname, NOHOST);
			getout = TRUE;
			errorlevel = 1;
		} /* else */

	} /* while */

} /* getinfo */


#ifdef PM_APP
void whoamiThread(void *x)
{

	getinfo();
	WinPostMsg(hwndClient, UM_WHOAMI_DONE, NULL, NULL);
	_endthread();
} /* whoamiThread */


void fontxy(HPS hps, INT *x, INT *y)
/* FONTMETRICS is a HUGE structure...rather than keep that whole thing in
 *  memory for the two elements I need, I just store it temporarily on the
 *  stack, by using this separate function.
 */
{
	FONTMETRICS fm;

	GpiQueryFontMetrics(hps, sizeof (fm), &fm);
	*x = fm.lAveCharWidth;
	*y = fm.lMaxBaselineExt;
} /* fontxy */


MRESULT EXPENTRY whoamiWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
	static int tidWork;
	static INT cxChar, cyChar;
	HPS hps;
	RECTL rcl;

	switch (msg)
	{
		case WM_CREATE:
			tidWork = _beginthread(whoamiThread, NULL, 32768, (void *) NULL);
			if (tidWork == -1)
				WinPostMsg(hwnd, WM_QUIT, NULL, NULL);
			return(0);

		case UM_WHOAMI_DONE:
			hps = WinGetPS(hwnd);
			fontxy(hps, &cxChar, &cyChar);
			WinReleasePS(hps);                           

			WinSetWindowPos(hwndFrame, NULLHANDLE,
                			0, 0,
							cxChar * (strlen(myname) + ADDSPACE),
							cyChar * 3,
							SWP_SHOW | SWP_SIZE | SWP_MOVE);

			WinSetFocus(HWND_DESKTOP, hwndFrame);
			return(0);

		case WM_PAINT:
			hps = WinBeginPaint(hwnd, NULLHANDLE, NULL);

			WinQueryWindowRect(hwnd, &rcl);
			WinDrawText(hps, -1, myname, &rcl, CLR_NEUTRAL, CLR_BACKGROUND,
						DT_CENTER | DT_VCENTER | DT_ERASERECT);

			WinEndPaint(hps);
			return(0);
	} /* switch */

	return(WinDefWindowProc(hwnd, msg, mp1, mp2));
} /* whoamiWndProc */

#endif


int main(int argc, char **argv)
/*
 * The mainline. Ooh yeah.
 */
{

#ifdef PM_APP
	static CHAR  szClientClass [] = "PMwhoamiClass";
	static ULONG flFrameFlags = (FCF_TITLEBAR   | FCF_SYSMENU    |
								 FCF_SIZEBORDER | FCF_MINMAX     |
								 FCF_TASKLIST   | FCF_ICON        );
	HAB hab;
	HMQ hmq;
	QMSG qmsg;
#endif

	int looper;
	int exearg = 0;

	for (looper = 1; looper < argc; looper++)
	{
		if (stricmp(argv[looper], ARG_KEEPON) == 0)
			keepon = TRUE;
		else if (stricmp(argv[looper], ARG_JUSTHOST) == 0)
			justshow = JUSTSHOW_HOST;
		else if (stricmp(argv[looper], ARG_JUSTIP) == 0)
			justshow = JUSTSHOW_IP;
		else if (stricmp(argv[looper], ARG_EXE) == 0)
			shouldexe = TRUE;
		else
			bogusargs = TRUE;

		if (shouldexe)
		{
			exearg = looper + 1;
			looper = argc; /* break loop. */
		} /* if */
	} /* for */

#ifdef PM_APP

	hab = WinInitialize(0);
	hmq = WinCreateMsgQueue(hab, 0);

	WinRegisterClass(hab, szClientClass, whoamiWndProc, CS_SIZEREDRAW, 0);

	hwndFrame = WinCreateStdWindow(HWND_DESKTOP, WS_VISIBLE,
									&flFrameFlags, szClientClass,
									TITLEBAR, 0L, 0, 1, &hwndClient);

	while (WinGetMsg(hab, &qmsg, NULLHANDLE, 0, 0))
		WinDispatchMsg(hab, &qmsg);

	/* Deinitialize Presentation Manager stuff... */
	WinDestroyWindow(hwndFrame);
	WinDestroyMsgQueue(hmq);
	WinTerminate(hab);

#else
	getinfo();	

	if (shouldexe)
	{
		if (argv[exearg] == NULL)
			printf("%s\n", BOGUSARGS);
		else
		{
			iakenv = malloc(strlen(ENVSTRING) + strlen(myname) + 2);
			sprintf(iakenv, "%s=%s", ENVSTRING, myname);
			putenv(iakenv);
			spawnvp(P_NOWAIT, argv[exearg], &argv[exearg + 1]);
		} /* else */
	} /* if */
	else
		printf("%s\n", myname);
#endif

	return(errorlevel);
	
} /* main */

