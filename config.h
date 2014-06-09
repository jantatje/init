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
 * 
 * SInit configuration file
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* number of getties/logins */
#define NGETTY 1 /* should be less or equal to ten */
#define DEBUG /* comment out for non debug builds */
/* constants you might want to change, like the shell and/or getty */
const char shell[]	= "/bin/sh";
const char rc[]		= "/etc/rc";
const char getty[]	= "/sbin/agetty";

#endif /* #ifndef __CONFIG_H */

