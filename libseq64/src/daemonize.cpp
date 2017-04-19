/**
 * \file          daemonize.cpp
 * \library       sequencer64 application
 * \author        Chris Ahlstrom
 * \date          2005-07-03 to 2007-08-21
 * \updates       2017-04-19
 * \license       GNU GPLv2 or above
 *
 *  Daemonization module of the POSIX C Wrapper (PSXC) library
 *  Copyright (C) 2005-2017 by Chris Ahlstrom
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU General Public License along with
 *  this program; if not, write to the Free Software Foundation, Inc., 51
 *  Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *  Provides a function to make it easy to run an application as a (Linux)
 *  daemon.  There are large differences between POSIX daemons and Win32
 *  services.  Thus, this module is currently Linux-specific.
 *
 *  The quick-and-dirty story on creating a daemon.
 *
 *      -#  Fork off the parent process.
 *      -#  Change file mode mask (umask)
 *      -#  Open any logs for writing.
 *      -#  Create a unique Session ID (SID)
 *      -#  Change the current working directory to a safe place.
 *      -#  Close standard file descriptors.
 *      -#  Enter actual daemon code.
 *
 *  This module handles all of the above.  Things not yet handled:
 *
 *      -#  Generation of various helpful files:
 *          -   PID file
 *          -   Lock file
 *          -   Error and information output file (though we do log some
 *              information via the syslog).
 *      -#  Thorough setting of the environment.
 *      -#  Thorough handling of user IDs and groups.
 *      -#  Redirection of the standard outputs to files.
 *
 *  See this project for an application that does all of the above:
 *
 *      https://github.com/bmc/daemonize
 *
 * \todo
 *      There is a service wrapper available under Win32.  It is called
 *      "srvhost.exe".  At this time, we *still* don't know how to use it, but
 *      it is available, and Windows XP seems to use it quite a bit.
 */

#include <string.h>                 /* strlen() etc.                        */
#include "daemonize.hpp"            /* daemonization functions & macros     */
#include "platform_macros.h"        /* TBD                                  */
#include "easy_macros.h"            /* used in the following macro call     */

#if SEQ64_HAVE_LIMITS_H
#include <limits.h>
#endif

#if SEQ64_HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if SEQ64_HAVE_SYSLOG_H
#include <syslog.h>                 /* syslog() and related constants       */
#endif

#if SEQ64_HAVE_SYS_TYPES_H
#include <sys/types.h>              /* for mode_t as used in this module    */
#endif

#if SEQ64_HAVE_UNISTD_H
#include <unistd.h>                 /* exit(), setsid()                     */
#endif

namespace seq64
{

/**
 *  Sets the current directory for the application.  A wrapper replacement for
 *  chdir() or _chdir().  It sets the current working directory in the
 *  application.  This function is necessary in order to make sure the current
 *  working directory is a safe place to work.
 *
 * \return
 *      Returns true if the path name is good, and the chdir() call succeeded.
 *      Otherwise, false is returned.
 *
 * \param path
 *      The full or relative name of the directory.
 */

bool
set_current_directory (const std::string & path)
{
    bool result = false;
    if (! path.empty())
    {
        int rcode = CHDIR(path.c_str());
        result = is_posix_success(rcode);
        if (! result)
        {
            errprintf("could not set current directory '%s'", path.c_str());
        }
    }
    return result;
}

/**
 *  Provides the path name of the current working directory.  This function is a
 *  wrapper for getcwd() and other such functions.  It obtains the current
 *  working directory in the application.
 *
 * \return
 *      The pointer to the string containg the name of the current directory.
 *      This name is the full path name for the directory.  If an error occurs,
 *      then an empty string is returned.
 */

std::string
get_current_directory ()
{
    std::string result;
    char temp[PATH_MAX];
    char * cwd = GETCWD(temp, PATH_MAX);  /* get current directory      */
    if (not_nullptr(cwd))
    {
      size_t len = strlen(cwd);
      if (len > 0)
         result = cwd;
      else
      {
         errprint("empty directory name returned");
      }
   }
   else
   {
      errprint("could not get current directory");
   }
   return result;
}

#ifdef PLATFORM_POSIX_API

/**
 *
 *    Performs a number of actions needed by a UNIX daemon.
 *
 *    These actions are layed out in the following URLs.
 *
 *       http://www.linuxprofilm.com/articles/linux-daemon-howto.html#s1   <br>
 *       http://www.deez.info/sengelha/projects/sigrandd/doc/#5.           <br>
 *
 *       -# Fork off the parent process.
 *       -# Change file mode mask (umask).
 *       -# Open the system log for writing (optional).
 *       -# Create a unique Session ID (SID).
 *       -# Open any logs for writing.
 *       -# Change the current working directory to a safe place.
 *       -# Close standard file descriptors.
 *       -# Enter actual daemon code.
 *       -# Close the log upon exiting the daemon.
 *
 * \algorithm
 *    -# Fork an identical child process.  After the fork() call, we now
 *       have a copy of this application running as a child process.  We can
 *       then kill off the parent process using exit().  The child inherits
 *       the process group ID of the parent but gets a new process ID, so
 *       we're guaranteed that the child is not a process group leader. This
 *       is a prerequisite for the call to setsid() that is done later.
 *       [Note:  Of interest is the Linux clone() call.]
 *    -# Once this succeeds, we want to change the file-mode mask so that
 *       the daemon has access to system files that it creates.  The file
 *       mode creation mask that's inherited could have been set to deny
 *       certain permissions.
 *    -# Optional:  Open the system log for writing.  We make
 *       the system log our error-log, using the set_syslogging()
 *       function of the errorlogging.c module.
 *    -# The child process must get a unique SID (session ID) from the
 *       kernel in order to operate.  Otherwise, the child process becomes
 *       an orphan in the system. The pid_t type is used for the new SID.
 *       The process becomes a session leader of a new session, becomes the
 *       process group leader of a new process group, and has no controlling
 *       terminal.
 *    -# The current working directory should be changed to some place that
 *       is guaranteed to always be there. Since many Linux distributions do
 *       not completely follow the Linux Filesystem Hierarchy standard, the
 *       only directory that is guaranteed to be there is the root (/).
 *       However, we make it a command-line option that defaults to ".".
 *       See the functions in the audio_arguments.c/h module.
 *       Since daemons normally exist until the system is rebooted, if the
 *       daemon stays on a mounted filesystem, that filesystem cannot be
 *       unmounted.
 *    -# An important step in setting up a daemon is closing out the
 *       standard file descriptors (STDIN, STDOUT, STDERR). Since a daemon
 *       cannot use the terminal, these file descriptors are useless, and a
 *       potential security hazard.
 *    -# As the last step, we check out the command-line arguments and the
 *       audio parameters, and use them to start the desired task.  Note:
 *       Since this is a daemon, only the server commands will be supported.
 *    -# If the server is stopped normally, we go ahead and call closelog(),
 *       even though it is optional.
 *
 * \todo
 *    Try to figure out why 4 (instead of two) audiotest items come up in
 *    the process list (in an unrelated project).
 *
 * \param appname
 *    Name of the application to daemonize.
 *
 * \param cwd
 *    Current working directory to set.
 *
 * \return
 *    The previous umask is returned, and should be saved for later
 *    restoration.
 */

uint32_t
daemonize (const std::string & appname, const std::string & cwd)
{
   static std::string s_app_name;            /* to survive forking?           */
   uint32_t result = 0;
   s_app_name.clear();                       /* blank out the base app name   */
   if (! appname.empty())
      s_app_name = appname;                  /* copy the base app name        */

   pid_t pid = fork();                       /* 1. fork the parent process    */
   if (is_posix_error(pid))                  /*    -1 process creation failed */
   {
      errprint("fork() failed");
      exit(EXIT_FAILURE);                    /*    exit parent as a failure   */
   }
   else if (pid > 0)                         /*    process creation succeeded */
   {
      exit(EXIT_SUCCESS);                    /*    exit parent successfully   */
   }
   else                                      /*    now we're in child process */
   {
      bool cwdgood = ! cwd.empty();
      result = (uint32_t) umask(0);          /* 2. save and set the user mask */
      pid_t sid = setsid();                  /* 3. get a new session ID       */
      if (sid < 0)                           /*    couldn't get one           */
         exit(EXIT_FAILURE);                 /*    exit the child process     */

      if (s_app_name.empty())
         s_app_name = "bad daemon";

      openlog(s_app_name.c_str(), LOG_CONS|LOG_PID, LOG_USER); /* 4. log it   */
      if (cwdgood)
         cwdgood = cwd != ".";               /*    don't bother with "."      */

      if (cwdgood)
      {
         if (! set_current_directory(cwd))
            exit(EXIT_FAILURE);              /*    bug out royally!           */
      }
      close(STDIN_FILENO);                   /* 6. close standard files       */
      close(STDOUT_FILENO);
      close(STDERR_FILENO);
      syslog(LOG_NOTICE, "seq64 daemon started");
   }
   return result;
}

/*
 *    This function undoes the daemon setup.  It undoes the actions of the
 *    daemonize() function by first restoring the previous umask.  Then, it
 *    restores the error-level of the application to the default error-level
 *    ("--error").  Not sure how useful this function is, since the daemon is
 *    probably exiting anyway.
 *
 * \param previous_umask
 *    Previous umask value, for later restoring.
 */

void
undaemonize (uint32_t previous_umask)
{
   syslog(LOG_NOTICE, "daemon exited");
   closelog();
   if (previous_umask != 0)
      (void) umask(previous_umask);          /* restore user mask             */
}

#else                               /* Win32                                  */

#error The daemonize module is currently not supported under Win32

/**
 * \todo
 *    Implement "daemonizing" for Windows, including redirection to the
 *    Windows Event Log.  Still need to figure out a way to do this very
 *    simply, a la' Microsoft's 'svchost' executable.
 */

#endif   // PLATFORM_POSIX vs Win32

}        // namespace seq64

/*
 * vim: ts=4 sw=4 et ft=cpp
 */
