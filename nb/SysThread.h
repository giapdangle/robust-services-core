//==============================================================================
//
//  SysThread.h
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#ifndef SYSTHREAD_H_INCLUDED
#define SYSTHREAD_H_INCLUDED

#include "Permanent.h"
#include <bitset>
#include <cstddef>
#include <memory>
#include "Clock.h"
#include "SysDecls.h"
#include "SysTypes.h"

namespace NodeBase
{
   class Thread;
}

//------------------------------------------------------------------------------

namespace NodeBase
{
//  Operating system abstraction layer: native thread wrapper.
//
class SysThread : public Permanent
{
   friend std::unique_ptr< SysThread >::deleter_type;
   friend class Thread;
   friend class Orphans;
public:
   //  Returns the native identifier of the running thread.
   //
   static SysThreadId RunningThreadId();

   //  Thread priorities.
   //
   enum Priority
   {
      LowPriority,       // preemptable threads
      DefaultPriority,   // unpreemptable threads
      SystemPriority,    // InitThread
      WatchdogPriority,  // RootThread
      Priority_N         // number of priorities
   };
private:
   //  The signature of the Thread entry function.
   //
   typedef main_t (*ThreadEntry) (void* self);

   //  The signature of a signal handler.
   //
   typedef void (*sighandler_t) (signal_t sig);

   //  The thread's status flags.
   //
   enum StatusFlag
   {
      SetPriorityFailed,  // failed to set priority
      StackOverflowed,    // caused SIGSTACK1
      IsExiting,          // is about to return
      StatusFlag_N        // number of flags
   };

   typedef std::bitset< StatusFlag_N > StatusFlags;

   //  Creates a native thread for CLIENT.  ENTRY is its entry function,
   //  PRIO is the priority at which it will run, and SIZE is its stack
   //  size (a size of 0 uses the default size).
   //
   SysThread(const Thread* client,
      const ThreadEntry entry, Priority prio, size_t size);

   //  Wraps an existing native thread.  Used to create RootThread.
   //
   SysThread();

   //  Releases resources.
   //
   ~SysThread();

   //  Used by the constructor to create an actual native thread.  ENTRY,
   //  CLIENT, and SIZE were passed to the constructor.  Updates NID to
   //  the new thread's native identifier.  Returns the thread's native
   //  handle.
   //
   static SysThread_t Create(const ThreadEntry entry,
      const Thread* client, size_t size, SysThreadId& nid);

   //  Used by the constructor to wrap the thread that is running main().
   //  Returns the thread's native handle after possibly performing some
   //  platform-specific work.
   //
   static SysThread_t Wrap();

   //  Deletes a native thread and nullifies its handle.
   //
   static void Delete(SysThread_t& thread);

   //  Creates a sentry.  A thread waits on a sentry, and other threads
   //  signal it to wake the thread up.
   //
   static SysSentry_t CreateSentry();

   //  Deletes a sentry and nullifies its handle.
   //
   static void DeleteSentry(SysSentry_t& sentry);

   //  Returns the thread's native identifier.
   //
   SysThreadId Nid() const { return nid_; }

   //  Performs environment-specific actions upon entering the thread.
   //  Returns a non-zero value if the thread should immediately return.
   //
   signal_t Start();

   //  Sleeps for MSECS (0 = yield, -1 = infinite).  The outcomes are
   //  o Failed: probably an obscure but serious bug
   //  o Interrupted: was awoken before the requested duration elapsed
   //  o Completed: slept for the requested duration
   //
   DelayRc Delay(msecs_t msecs);

   //  Signals a thread's sentry.  If the thread is delaying, it awakens.
   //  If it is not delaying, it only yields (sleeps for zero seconds,
   //  allowing other threads to run) the next time it delays.
   //
   bool Interrupt();

   //  Sets or changes the thread's priority.
   //
   bool SetPriority(Priority prio);

   //  Registers HANDLER against SIG.
   //
   static void RegisterForSignal(signal_t sig, sighandler_t handler);

   //  Overridden to display member variables.
   //
   virtual void Display(std::ostream& stream,
      const std::string& prefix, const Flags& options) const override;

   //  Overridden for patching.
   //
   virtual void Patch(sel_t selector, void* arguments) override;

   //  Overridden to prohibit copying.
   //
   SysThread(const SysThread& that);
   void operator=(const SysThread& that);

   //  Reference to the native thread.
   //
   SysThread_t nthread_;

   //  Native identifier for this thread.
   //
   SysThreadId nid_;

   //  The thread's current status.
   //
   StatusFlags status_;

   //  A reference to a native object that is waited on to implement Delay.
   //
   SysSentry_t sentry_;

   //  The signal that caused the thread to be deleted.
   //
   const signal_t signal_;
};
}
#endif