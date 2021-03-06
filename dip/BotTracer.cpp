//==============================================================================
//
//  BotTracer.cpp
//
//  Copyright (C) 2017  Greg Utas
//
//  Diplomacy AI Client - Part of the DAIDE project (www.daide.org.uk).
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
#include "BotTracer.h"
#include "Tool.h"
#include "Debug.h"
#include "DipTypes.h"
#include "Singleton.h"
#include "SysTypes.h"

using namespace NodeBase;

//------------------------------------------------------------------------------

namespace Diplomacy
{
fixed_string BotTraceToolName = "BotTracer";
fixed_string BotTraceToolExpl = "traces Diplomacy messages";

class BotTraceTool : public Tool
{
   friend class Singleton< BotTraceTool >;
private:
   BotTraceTool() : Tool(DipTracer, 'd', true) { }
   c_string Name() const override { return BotTraceToolName; }
   c_string Expl() const override { return BotTraceToolExpl; }
};

//------------------------------------------------------------------------------

fn_name BotTracer_ctor = "BotTracer.ctor";

BotTracer::BotTracer()
{
   Debug::ft(BotTracer_ctor);

   Singleton< BotTraceTool >::Instance();
}
}
