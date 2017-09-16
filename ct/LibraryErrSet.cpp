//==============================================================================
//
//  LibraryErrSet.cpp
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#include "LibraryErrSet.h"
#include <cstring>
#include <iosfwd>
#include <sstream>
#include "CliBuffer.h"
#include "Debug.h"
#include "Formatters.h"

using std::ostream;
using std::string;

//------------------------------------------------------------------------------

namespace CodeTools
{
fn_name LibraryErrSet_ctor = "LibraryErrSet.ctor";

LibraryErrSet::LibraryErrSet(const string& name, LibExprErr err, size_t pos) :
   LibrarySet(name),
   err_(err),
   pos_(pos)
{
   Debug::ft(LibraryErrSet_ctor);
}

//------------------------------------------------------------------------------

fn_name LibraryErrSet_dtor = "LibraryErrSet.dtor";

LibraryErrSet::~LibraryErrSet()
{
   Debug::ft(LibraryErrSet_dtor);
}

//------------------------------------------------------------------------------

fn_name LibraryErrSet_Check = "LibraryErrSet.Check";

word LibraryErrSet::Check(ostream& stream, string& expl) const
{
   Debug::ft(LibraryErrSet_Check);

   return Error(expl);
}

//------------------------------------------------------------------------------

fn_name LibraryErrSet_Count = "LibraryErrSet.Count";

word LibraryErrSet::Count(string& result) const
{
   Debug::ft(LibraryErrSet_Count);

   return Error(result);
}

//------------------------------------------------------------------------------

fn_name LibraryErrSet_Countlines = "LibraryErrSet.Countlines";

word LibraryErrSet::Countlines(string& result) const
{
   Debug::ft(LibraryErrSet_Countlines);

   return Error(result);
}

//------------------------------------------------------------------------------

void LibraryErrSet::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   LibrarySet::Display(stream, prefix, options);

   stream << prefix << "err : " << err_ << CRLF;
   stream << prefix << "pos : " << pos_ << CRLF;
}

//------------------------------------------------------------------------------

word LibraryErrSet::Error(string& expl) const
{
   if(err_ == InterpreterError)
   {
      expl = strError(err_);
      return -7;
   }

   std::ostringstream stream;

   auto pointer = CliBuffer::ErrorPointer;
   stream << spaces(pos_ - strlen(pointer)) << pointer << CRLF;
   stream << spaces(2) << strError(err_);

   expl = stream.str();
   return -2;
}

//------------------------------------------------------------------------------

fn_name LibraryErrSet_Format = "LibraryErrSet.Format";

word LibraryErrSet::Format(string& expl) const
{
   Debug::ft(LibraryErrSet_Format);

   return Error(expl);
}

//------------------------------------------------------------------------------

fn_name LibraryErrSet_List = "LibraryErrSet.List";

word LibraryErrSet::List(ostream& stream, string& expl) const
{
   Debug::ft(LibraryErrSet_List);

   return Error(expl);
}

//------------------------------------------------------------------------------

fn_name LibraryErrSet_Parse = "LibraryErrSet.Parse";

word LibraryErrSet::Parse(string& expl, const string& opts) const
{
   Debug::ft(LibraryErrSet_Parse);

   return Error(expl);
}

//------------------------------------------------------------------------------

fn_name LibraryErrSet_PreAssign = "LibraryErrSet.PreAssign";

word LibraryErrSet::PreAssign(string& expl) const
{
   Debug::ft(LibraryErrSet_PreAssign);

   return Error(expl);
}

//------------------------------------------------------------------------------

fn_name LibraryErrSet_Scan = "LibraryErrSet.Scan";

word LibraryErrSet::Scan
   (ostream& stream, const string& pattern, string& expl) const
{
   Debug::ft(LibraryErrSet_Scan);

   return Error(expl);
}

//------------------------------------------------------------------------------

fn_name LibraryErrSet_Show = "LibraryErrSet.Show";

word LibraryErrSet::Show(string& list) const
{
   Debug::ft(LibraryErrSet_Show);

   return Error(list);
}

//------------------------------------------------------------------------------

fn_name LibraryErrSet_Sort = "LibraryErrSet.Sort";

word LibraryErrSet::Sort(ostream& stream, string& expl) const
{
   Debug::ft(LibraryErrSet_Sort);

   return Error(expl);
}

//------------------------------------------------------------------------------

fn_name LibraryErrSet_Trim = "LibraryErrSet.Trim";

word LibraryErrSet::Trim(ostream& stream, string& expl) const
{
   Debug::ft(LibraryErrSet_Trim);

   return Error(expl);
}
}