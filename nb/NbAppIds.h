//==============================================================================
//
//  NbAppIds.h
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
#ifndef NBAPPIDS_H_INCLUDED
#define NBAPPIDS_H_INCLUDED

#include "SysTypes.h"

//------------------------------------------------------------------------------

namespace NodeBase
{
//  Object pool identifiers.  A new object pool must define an identifier here.
//  Fixed identifiers are required so that it would be possible to serialize an
//  object in one software release and import it into another release.
//
enum ObjectPoolIds
{
   ThreadObjPoolId = 1,
   MsgBufferObjPoolId = 2,
   IpBufferObjPoolId = 3,
   SbIpBufferObjPoolId = 4,
   BtIpBufferObjPoolId = 5,
   ContextObjPoolId = 6,
   MessageObjPoolId = 7,
   MsgPortObjPoolId = 8,
   ProtocolSMObjPoolId = 9,
   TimerObjPoolId = 10,
   EventObjPoolId = 11,
   ServiceSMObjPoolId = 12,
   MediaEndptObjPoolId = 13,
   DipIpBufferObjPoolId = 14
};

//------------------------------------------------------------------------------
//
//  Reserved software debugging flags.  Ad hoc usage should begin with the
//  last FlagId defined here.
//
constexpr FlagId ShowToolProgress = 0;
constexpr FlagId DisableRootThreadFlag = 1;
constexpr FlagId ThreadReenterFlag = 2;
constexpr FlagId ThreadRecoverTrapFlag = 3;
constexpr FlagId ThreadCtorTrapFlag = 4;
constexpr FlagId ThreadCtorRetrapFlag = 5;
constexpr FlagId ThreadRetrapFlag = 6;
constexpr FlagId CipAlwaysOverIpFlag = 7;
constexpr FlagId CallTrapFlag = 8;
constexpr FlagId CipIamTimeoutFlag = 9;
constexpr FlagId CipAlertingTimeoutFlag = 10;
constexpr FlagId FirstAppDebugFlag = 11;
}
#endif
