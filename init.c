/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Jan Tatje
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * minimalistic init: SInit (initializing your system with just 140 SLOC)
 * 
 * What this does:
 * 	1. be process one
 *	2. execute rc scripts to setup system
 * 	3. wait() for zombies
 * 
 * init should only do the most basic tasks to make the system useable.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/reboot.h>
#include <string.h>

#include "config.h"

/*define actions for shutdown routine*/
enum shutdown_action
{
	HALT,
	REBOOT
};

void shutdown(enum shutdown_action action);
int spawn_getty(int i);
void handle_signal(int signal);

const char tty[]	= "/dev/tty%i";
const char minus[]	= "-";

int getty_ids[NGETTY] = {0};

int main(int argc, char *argv[])
{
	int pid, ret, found, i;
	int status;

	/*Say hello from init*/
	puts("init: SInit");

	/*are we root?
	  are we process one?*/
	#ifndef DEBUG
	if ( getuid() != 0 )
	{
		puts("init: only root can execute init");
		return 1;
	}
	if ( getpid() != 1 )
	{
		puts("init: not process 1");
		return 1;
	}
	#endif

	/*setup signal handlers*/
	signal(SIGINT, &handle_signal);
	signal(SIGTERM, &handle_signal);

	/*deal with basic init*/
	setsid(); /*set session id*/

	/*execute rc script*/
	/*rc script must be a file, no folder is accepted,
	  the script can execute files from rc.d if it wants to though*/
	puts("init: executing rc script");
	pid = fork();
	if ( pid == 0 )
	{
		execl(shell, shell, rc, NULL);
		puts("init: exec failed");
		_exit(1); /*exit child process*/
	}
	if ( pid == -1 )
	{
		puts("init: failed to run rc script");
		shutdown(REBOOT);
	}
	else
	{
		waitpid(pid, &status, 0);
		if (WEXITSTATUS(status)) 
		{
			puts("init: rc returned != 0, rebooting"); 
			shutdown(REBOOT);
		}
	}

	/*spawn getty/login (Important!)*/
	for ( i=0; i < NGETTY; i++ )
		getty_ids[i] = spawn_getty(i);

	/*wait for zombies*/
	for (;;)
	{
		found = 0;
		ret = wait(NULL);
		for ( i = 0; i < NGETTY; i++ )
		{
			if (ret == getty_ids[i] || getty_ids[i] == -1)
			{
				getty_ids[i] = spawn_getty(i);
				found = 1;
				break;
			}
		}
		if (found)
			continue;
		else if (ret > 0)
			puts("init: killed a zombie!");
	}

	return 1; /*something went wrong*/
}

int spawn_getty(int ttyn)
{
	/*returns -1 on failure, pid on success*/
	int pid, ret;
	char ttyc[256]; 
	/*copy tty string and replace number*/
	ret = snprintf(ttyc, (sizeof ttyc/sizeof *ttyc), tty, ttyn);
	if ( ret >= (sizeof ttyc/sizeof *ttyc) ) /*string was truncated, fail.*/
		return -1;

	printf("init: spawning getty on %s\n", ttyc);
	pid = fork();
	if ( pid == 0 )
	{
		execl(getty, getty, ttyc+5, NULL); /* + 5 to remove /dev/ for agetty */
		_exit(1); /*exit child process*/
	}

	return pid;
}

void shutdown(enum shutdown_action action)
{
	/*calls the init script to halt all services, syncs, then halts*/
	int pid;
	
	#ifdef DEBUG
	_exit(0);
	#endif
	/*tell rc scripts to stop all services.*/
	/*bsd calls /etc/rc with shutdown argument, we're gonna do the same*/
	pid = fork();
	if (pid == 0)
		execl(shell, shell, rc, "shutdown", NULL);
	/*sync to prevent data loss!*/
	sync();
	/*halt here*/
	switch( action )
	{
		case HALT:
			puts("init: halting...");
			reboot(RB_POWER_OFF);
			break;
		case REBOOT:
			puts("init: rebooting...");
			reboot(RB_AUTOBOOT);
			break;
		default:
			_exit(1);
	}
}

void handle_signal(int signal)
{
	switch( signal )
	{
		case SIGINT:	/*reboot*/
			shutdown(REBOOT);
		case SIGTERM:	/*others killall and go to singleuser -> halt for us?*/
			shutdown(HALT);
		default:		/*ignore all other signals*/
			break;
	}
}

