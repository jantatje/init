/*
	minimalistic init: SInit (initializing your system with just 140 SLOC)
	
	What this does:
		1. be process one
		2. execute rc scripts to setup system
		3. wait() for zombies

	init should only do the most basic tasks to make the system useable.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/reboot.h>

#include "config.h"

/*define actions for halt routine*/
enum halt_action
{
	HALT,
	REBOOT
};

const char tty[]	= "/dev/tty8";
const char minus[]	= "-";

void halt(enum halt_action action);
int spawn_getty(int i);
void handle_signal(int signal);

int main(int argc, char *argv[])
{
	int pid, ret, i;
	int getty_ids[NGETTY] = {0};
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
	
	/*deal with basic init*/
	/*close(0);
	close(1);*/
	setsid(); /*set session id*/

	/*execute rc script*/
	/*rc script must be a file, no folder is accepted,
	  the script can execute files from rc.d if it wants to though*/
	puts("init: executing rc script");
	pid = fork();
	if ( pid == 0 )
	{
		/*dup(0);*/
		execl(shell, shell, rc, NULL);
		puts("init: exec failed");
		_exit(1); /*exit child process*/
	}
	if ( pid == -1 )
	{
		puts("init: failed to run rc script");
		halt(REBOOT);
	}
	else 
	{
		waitpid(pid, &status, 0);
		if (WEXITSTATUS(status)) 
		{
			puts("init: rc returned != 0"); 
			halt(REBOOT);
		}
	}

	/*spawn getty/login (Important!)*/
	for ( i=0; i < NGETTY; i++ )
		getty_ids[i] = spawn_getty(i);

	/*wait for zombies*/
	for(;;)
	{
		ret = wait(NULL);
		for (i=0; i < NGETTY; i++)
		{
			if (ret == getty_ids[i] || getty_ids[i] == -1)
				getty_ids[i] = spawn_getty(i);
			else
				puts("init: killed a zombie!");
		}
	}

	return 1; /*something went wrong*/
}

int spawn_getty(int tty8)
{
	int pid, i;
	/*TODO: fix this mess, allow more than 10 gettys*/
	char ttyc[10];
	/*copy tty string and replace number*/
	i = 0;
	while(tty[i]) 
	{
		ttyc[i] = tty[i];
		i++;
	}
	ttyc[8] = (char)(tty8 + '0');
	ttyc[9] = '\0';

	printf("init: spawning getty on %s\n", ttyc);
	pid = fork();
	if (pid == 0)
	{
		execl(getty, getty, ttyc+5, NULL); /* + 5 to remove /dev/ for agetty */
		_exit(1); /*exit child process*/
	}

	return pid;
}

void halt(enum halt_action action)
{
	#ifdef DEBUG
	_exit(0);
	#endif
	/*tell rc scripts to stop all services.*/
	/*bsd calls /etc/rc with shutdown argument, we're gonna do the same*/
	execl(shell, shell, rc, "shutdown", NULL);
	/*sync to prevent data loss!*/
	sync();
	/*halt here*/
	switch(action)
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
			puts("init: this shouldn't happen, panic time.");
			_exit(1);
	}
	_exit(1);
}

void handle_signal(int signal)
{
	switch(signal)
	{
		case SIGHUP:	/*clean ttys*/
			puts("init: sorry, handling for SIGHUP not yet implemented");
			break;
		case SIGINT:	/*reboot*/
			halt(REBOOT);
		case SIGTERM:	/*others killall and go to singleuser -> halt for us?*/
			halt(HALT);
		case SIGTSTP:	/*block login*/
			puts("init: sorry, handling for SIGTSTP not yet implemented");
			/*kill all gettys and prevent them from restarting by setting their
			 *pids in the getty pid array to ourself (1)*/
			break;
		default:
			puts("init: unhandled signal recieved.");
			break;
	}
}

