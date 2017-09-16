//==============================================================================
//
//  Tool.cpp
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#include "Tool.h"
#include <cctype>
#include <ostream>
#include "Algorithms.h"
#include "Debug.h"
#include "Element.h"
#include "Singleton.h"
#include "ToolRegistry.h"
#include "TraceBuffer.h"

using std::ostream;
using std::string;

//------------------------------------------------------------------------------

namespace NodeBase
{
fn_name Tool_ctor = "Tool.ctor";

Tool::Tool(FlagId tid, char abbr, bool safe) :
   abbr_(abbr),
   safe_(safe)
{
   Debug::ft(Tool_ctor);

   tid_.SetId(tid);
   Singleton< ToolRegistry >::Instance()->BindTool(*this);
}

//------------------------------------------------------------------------------

fn_name Tool_dtor = "Tool.dtor";

Tool::~Tool()
{
   Debug::ft(Tool_dtor);

   Singleton< ToolRegistry >::Instance()->UnbindTool(*this);
}

//------------------------------------------------------------------------------

ptrdiff_t Tool::CellDiff()
{
   int local;
   auto fake = reinterpret_cast< const Tool* >(&local);
   return ptrdiff(&fake->tid_, fake);
}

//------------------------------------------------------------------------------

void Tool::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   Immutable::Display(stream, prefix, options);

   stream << prefix << "tid  : " << Tid() << CRLF;
   if(isprint(abbr_))
   {
      stream << prefix << "abbr : " << abbr_ << CRLF;
   }
   stream << prefix << "safe : " << safe_ << CRLF;
}

//------------------------------------------------------------------------------

fn_name Tool_Expl = "Tool.Expl";

const char* Tool::Expl() const
{
   Debug::SwErr(Tool_Expl, Tid(), 0);
   return EMPTY_STR;
}

//------------------------------------------------------------------------------

fn_name Tool_IsSafe = "Tool.IsSafe";

bool Tool::IsSafe() const
{
   Debug::ft(Tool_IsSafe);

   if(safe_) return true;
   return Element::RunningInLab();
}

//------------------------------------------------------------------------------

fn_name Tool_Name = "Tool.Name";

const char* Tool::Name() const
{
   Debug::SwErr(Tool_Name, Tid(), 0);
   return EMPTY_STR;
}

//------------------------------------------------------------------------------

void Tool::Patch(sel_t selector, void* arguments)
{
   Immutable::Patch(selector, arguments);
}

//------------------------------------------------------------------------------

fixed_string ToolOn = "ON";
fixed_string ToolOff = "OFF";

string Tool::Status() const
{
   auto buff = Singleton< TraceBuffer >::Instance();
   return(buff->ToolIsOn(Tid()) ? ToolOn : ToolOff);
}
}