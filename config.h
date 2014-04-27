/*SInit configuration*/
#ifndef __CONFIG_H
#define __CONFIG_H

/*number of getties/logins*/
#define NGETTY 4 /*should be less or equal to ten*/
#define DEBUG /*comment out for non debug builds*/
/*constants you might want to change, like the shell and/or getty*/
const char shell[]	= "/bin/sh";
const char rc[]		= "/etc/rc";
const char getty[]	= "/sbin/agetty";

#endif /*#ifndef __CONFIG_H*/

