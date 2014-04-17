/*
	minimalistic init: SInit (shall not exceed 140 SLOC while doing everything it needs to do)
	This file is part of rbrc, an opensource ruby init suite.
	It was made for rbrc, has no dependencies on it though.

	What this does:
		1. be process one
		2. execute rc scripts to setup system (mounting devices???)
			(-start rbrc here)
		3. wait() for zombies

	init should only do the most basic tasks to make the system useable.
*/
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/reboot.h>

#define NGETTY 1 /*no more than one digit*/
#define DEBUG

const char shell[]	= "/bin/sh";
const char rc[]		= "/etc/rc";
const char getty[]	= "/sbin/agetty";
const char tty[]	= "/dev/tty8";
const char minus[]	= "-";

enum halt_action
{
	HALT,
	REBOOT
};

void halt(enum halt_action action);
void puts(const char *str);
int spawn_getty(int i);
void handle_signal(int signal);

int main(int argc, char *argv[])
{
	int pid, ret, i, fatal;
	int getty_ids[NGETTY] = {0};
	int status;
	fatal = 0;

	/*Say hello from init*/
	puts("init: SInit\n");

	/*are we root?
	  are we process one?*/
	if ( getuid() != 0 )
	{
		puts("init: only root can execute init\n");
		return 1;
	}
	if ( getpid() != 1 )
	{
		puts("init: not process 1\n");
		return 1;
	}

	/*setup signal handlers*/
	signal(SIGINT, &handle_signal);
	
	/*deal with basic init*/
	/*close(0);
	close(1);*/
	setsid();

	/*execute rc script*/
	/*rc script must be a file, no folder is accepted,
	  the script can execute files from rc.d if it wants to though*/
	puts("init: executing rc script\n");
	pid = fork();
	if ( pid == 0 )
	{
		/*dup(0);*/
		execl(shell, shell, rc, NULL);
		puts("init: exec failed\n");
		_exit(1); /*exit child process*/
	}
	if ( pid == -1 )
	{
		puts("init: failed to run rc script\n");
		//fatal = 1;
	}
	else 
	{
		waitpid(pid, &status, 0);
		if (WEXITSTATUS(status)) 
		{
			puts("init: rc returned != 0\n"); 
			//fatal = 1;
		}
	}

	/*i still did not decide if i want a singleuser mode, i tend to remove it*/
//	if (fatal)
//	{
//		int i;
//		char cd[] = " 1"; /*countdown string*/
//		/*if a fatal error occured drop into singleuser bash for now*/
//		puts("init: dropping to singleuser shell\n");
//		execl(shell, shell, minus, NULL);
//		puts("init: opening singleuser shell failed, rebooting in");
//		/*countdown*/
//		for (i=5; i > 0; i--)
//		{
//			cd[1] = '0'+i;
//			puts(cd);
//			sleep(1);
//		}
//		puts("\n");
//		halt(REBOOT);
//	}

	/*spawn getty/login (Important!)*/
	for ( i=0; i < NGETTY; i++ )
		getty_ids[i] = spawn_getty(i);

	/*wait for zombies*/
	while( (ret = wait(NULL)) )
	{
		for (i=0; i < NGETTY; i++)
		{
			if (ret == getty_ids[i] || getty_ids[i] == -1)
				getty_ids[i] = spawn_getty(i);
			else
				puts("init: killed a zombie!");
		}
	}

	return 1; /*or since we didn't crash completly should we just halt();?*/
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
	puts("init: spawning getty on "); puts(ttyc); puts("\n");
	pid = fork();
	if (pid == 0)
	{
		execl(getty, getty, ttyc+5, NULL); /* + 5 to remove /dev/ */
		_exit(1); /*exit child process*/
	}

	return pid;
}

void halt(enum halt_action action)
{
	#ifdef DEBUG
	_exit(1);
	#endif
	/*tell rbrc to stop all services.*/
	/*bsd calls /etc/rc with shutdown argument*/
	execl(shell, shell, rc, "shutdown", NULL);
	/*sync to prevent data loss!*/
	sync();
	/*halt here*/
	switch(action)
	{
		case HALT:
			puts("init: halting...\n");
			reboot(RB_POWER_OFF);
			break;
		case REBOOT:
			puts("init: rebooting...\n");
			reboot(RB_AUTOBOOT);
			break;
		default:
			puts("init: this shouldn't happen at all, panic time.\n");
			_exit(1234567890);
	}
	_exit(1);
}

void puts(const char *str)
{
	int len;

	for( len = 0; str[len]; len++ );
	write(1, str, len);
}

void handle_signal(int signal)
{
	switch(signal)
	{
		case SIGHUP:	/*clean ttys*/
			break;
		case SIGINT:	/*reboot*/
			halt(REBOOT);
		case SIGTERM:	/*killall and go singleuser -> halt for us?*/
			halt(HALT);
		case SIGTSTP:	/*block login*/
			/*kill all gettys and prevent them from restarting by setting their
			 *pids in the getty pid array to ourself (1)*/
			break;
		default:
			puts("init: unhandled signal recieved.\n");
			break;
	}
}

