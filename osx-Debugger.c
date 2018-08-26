//
//  Debugger.c
//  BWWorkspace
//
//  Created by Randall Maas on 10/19/14.
//  Copyright (c) 2014 Randall Maas. All rights reserved.
//

#include <sys/types.h>
#include <unistd.h>
#include <sys/sysctl.h>

// break into GDB code complied from following sources:
// http://blog.timac.org/?p=190
// http://cocoawithlove.com/2008/03/break-into-debugger.html

/* Returns true if the current process is being debugged (either
   running under the debugger or has a debugger attached post facto).
   https://developer.apple.com/library/mac/qa/qa1361/_index.html
 */
int AmIBeingDebugged(void)
{
   int                 junk;
   int                 mib[4];
   struct kinfo_proc   info;
   size_t              size;

   // Initialize the flags so that, if sysctl fails for some bizarre
   // reason, we get a predictable result.
   info.kp_proc.p_flag = 0;

   // Initialize mib, which tells sysctl the info we want, in this case
   // we're looking for information about a specific process ID.
   mib[0] = CTL_KERN;
   mib[1] = KERN_PROC;
   mib[2] = KERN_PROC_PID;
   mib[3] = getpid();

   // Call sysctl.
   size = sizeof(info);
   junk = sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);
   if (0u != junk)
      return 0;

   // We're being debugged if the P_TRACED flag is set.
   return (info.kp_proc.p_flag & P_TRACED);
}
