//==============================================================================
//
//  ObjectPoolAudit.cpp
//
//  Copyright (C) 2017  Greg Utas
//
//  This file is part of the Robust Services Core (RSC).
//
//  RSC is free software: you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software
//  Foundation, either version 3 of the License, or (at your option) any later
//  version.
//
//  RSC is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
//  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
//  details.
//
//  You should have received a copy of the GNU General Public License along
//  with RSC.  If not, see <http://www.gnu.org/licenses/>.
//
#include "ObjectPoolAudit.h"
#include <ostream>
#include <string>
#include "Debug.h"
#include "NbDaemons.h"
#include "ObjectPoolRegistry.h"
#include "Singleton.h"
#include "SysTypes.h"

using std::ostream;
using std::string;

//------------------------------------------------------------------------------

namespace NodeBase
{
fn_name ObjectPoolAudit_ctor = "ObjectPoolAudit.ctor";

ObjectPoolAudit::ObjectPoolAudit() :
   Thread(AuditFaction, Singleton< ObjectDaemon >::Instance()),
   interval_(5000),
   phase_(CheckingFreeq),
   pid_(NIL_ID)
{
   Debug::ft(ObjectPoolAudit_ctor);

   SetInitialized();
}

//------------------------------------------------------------------------------

fn_name ObjectPoolAudit_dtor = "ObjectPoolAudit.dtor";

ObjectPoolAudit::~ObjectPoolAudit()
{
   Debug::ft(ObjectPoolAudit_dtor);
}

//------------------------------------------------------------------------------

c_string ObjectPoolAudit::AbbrName() const
{
   return ObjectDaemonName;
}

//------------------------------------------------------------------------------

fn_name ObjectPoolAudit_Destroy = "ObjectPoolAudit.Destroy";

void ObjectPoolAudit::Destroy()
{
   Debug::ft(ObjectPoolAudit_Destroy);

   Singleton< ObjectPoolAudit >::Destroy();
}

//------------------------------------------------------------------------------

void ObjectPoolAudit::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   Thread::Display(stream, prefix, options);

   stream << prefix << "interval : " << interval_ << CRLF;
   stream << prefix << "phase    : " << phase_ << CRLF;
   stream << prefix << "pid      : " << int(pid_) << CRLF;
}

//------------------------------------------------------------------------------

fn_name ObjectPoolAudit_Enter = "ObjectPoolAudit.Enter";

void ObjectPoolAudit::Enter()
{
   Debug::ft(ObjectPoolAudit_Enter);

   //  Audit blocks forever.
   //
   while(true)
   {
      Pause(interval_);
      Singleton< ObjectPoolRegistry >::Instance()->AuditPools();
   }
}

//------------------------------------------------------------------------------

void ObjectPoolAudit::Patch(sel_t selector, void* arguments)
{
   Thread::Patch(selector, arguments);
}

//------------------------------------------------------------------------------

fn_name ObjectPoolAudit_SetInterval = "ObjectPoolAudit.SetInterval";

void ObjectPoolAudit::SetInterval(msecs_t interval)
{
   Debug::ft(ObjectPoolAudit_SetInterval);

   auto prev = interval_;
   interval_ = interval;

   //  If the thread was sleeping forever and has now been enabled, wake it.
   //
   if((prev == TIMEOUT_NEVER) && (interval_ != TIMEOUT_NEVER)) Interrupt();
}
}
