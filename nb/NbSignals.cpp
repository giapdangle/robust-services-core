//==============================================================================
//
//  NbSignals.cpp
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#include "NbSignals.h"
#include "Debug.h"
#include "PosixSignal.h"
#include "Singleton.h"
#include "SysSignals.h"
#include "SysTypes.h"

//------------------------------------------------------------------------------

namespace NodeBase
{
//  Signals defined for use within NodeBase.  They cannot be used with the
//  functions signal() and raise(), but their SIG... constants can be used
//  with SignalException.
//
class SigClose : public PosixSignal
{
   friend class Singleton< SigClose >;
private:
   SigClose();
};

class SigYield : public PosixSignal
{
   friend class Singleton< SigYield >;
private:
   SigYield();
};

class SigTraps : public PosixSignal
{
   friend class Singleton< SigTraps >;
private:
   SigTraps();
};

class SigRetrap : public PosixSignal
{
   friend class Singleton< SigRetrap >;
private:
   SigRetrap();
};

class SigStack1 : public PosixSignal
{
   friend class Singleton< SigStack1 >;
private:
   SigStack1();
};

class SigStack2 : public PosixSignal
{
   friend class Singleton< SigStack2 >;
private:
   SigStack2();
};

class SigPurge : public PosixSignal
{
   friend class Singleton< SigPurge >;
private:
   SigPurge();
};

class SigDeleted : public PosixSignal
{
   friend class Singleton< SigDeleted >;
private:
   SigDeleted();
};

//------------------------------------------------------------------------------

SigClose::SigClose() : PosixSignal(SIGCLOSE, "SIGCLOSE",
   "Non-Error Shutdown", 12, PS_Interrupt() |
   PS_Delayed() | PS_Exit() | PS_Final() | PS_NoLog() | PS_NoError()) { }

SigYield::SigYield() : PosixSignal(SIGYIELD, "SIGYIELD",
   "Running Locked Too Long", 4, Flags()) { }

SigTraps::SigTraps() : PosixSignal(SIGTRAPS, "SIGTRAPS",
   "Trap Threshold Exceeded", 0, PS_Exit()) { }

SigRetrap::SigRetrap() : PosixSignal(SIGRETRAP, "SIGRETRAP",
   "Trap During Recovery", 0, PS_NoRecover() | PS_Exit()) { }

SigStack1::SigStack1() : PosixSignal(SIGSTACK1, "SIGSTACK1",
   "Stack Overflow: Attempt Recovery", 0, Flags()) { }

SigStack2::SigStack2() : PosixSignal(SIGSTACK2, "SIGSTACK2",
   "Stack Overflow: Exit and Recreate", 0, PS_NoRecover() | PS_Exit()) { }

SigPurge::SigPurge() : PosixSignal(SIGPURGE, "SIGPURGE",
   "Suicided [errval = 0] or Killed [errval > 0]", 16,
   PS_Interrupt() | PS_Exit() | PS_Final()) { }

SigDeleted::SigDeleted() : PosixSignal(SIGDELETED, "SIGDELETED",
   "Thread Deleted", 0, PS_Exit() | PS_Final()) { }

//------------------------------------------------------------------------------

fn_name NodeBase_CreatePosixSignals = "NodeBase.CreatePosixSignals";

void CreatePosixSignals()
{
   Debug::ft(NodeBase_CreatePosixSignals);

   //  Create this platform's native signals and then our proprietary signals.
   //
   SysSignals::CreateNativeSignals();

   Singleton< SigClose >::Instance();
   Singleton< SigYield >::Instance();
   Singleton< SigTraps >::Instance();
   Singleton< SigRetrap >::Instance();
   Singleton< SigStack1 >::Instance();
   Singleton< SigStack2 >::Instance();
   Singleton< SigPurge >::Instance();
   Singleton< SigDeleted >::Instance();
}
}