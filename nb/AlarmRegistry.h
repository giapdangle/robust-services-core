//==============================================================================
//
//  AlarmRegistry.h
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
#ifndef ALARMREGISTRY_H_INCLUDED
#define ALARMREGISTRY_H_INCLUDED

#include "Dynamic.h"
#include <string>
#include "NbTypes.h"
#include "Registry.h"
#include "SysTypes.h"

namespace NodeBase
{
   class Alarm;
}

//------------------------------------------------------------------------------

namespace NodeBase
{
//  Global registry for alarms.
//
class AlarmRegistry : public Dynamic
{
   friend class Singleton< AlarmRegistry >;
   friend class Alarm;
public:
   //> The maximum number of alarms.
   //
   static const id_t MaxAlarms;

   //  Returns the alarm associated with NAME.
   //
   Alarm* Find(const std::string& name) const;

   //  Returns the registry.
   //
   const Registry< Alarm >& Alarms() const { return alarms_; }

   //  Overridden for restarts.
   //
   void Shutdown(RestartLevel level) override;

   //  Overridden to display member variables.
   //
   void Display(std::ostream& stream,
      const std::string& prefix, const Flags& options) const override;

   //  Overridden for patching.
   //
   void Patch(sel_t selector, void* arguments) override;
private:
   //  Private because this singleton is not subclassed.
   //
   AlarmRegistry();

   //  Private because this singleton is not subclassed.
   //
   ~AlarmRegistry();

   //  Registers GROUP.
   //
   bool BindAlarm(Alarm& alarm);

   //  Removes ALARM from the registry.
   //
   void UnbindAlarm(Alarm& alarm);

   //  The registry of alarms.
   //
   Registry< Alarm > alarms_;
};
}
#endif
