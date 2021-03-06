//==============================================================================
//
//  CfgParmRegistry.h
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
#ifndef CFGPARMREGISTRY_H_INCLUDED
#define CFGPARMREGISTRY_H_INCLUDED

#include "Protected.h"
#include <cstddef>
#include <iosfwd>
#include <string>
#include <vector>
#include "NbTypes.h"
#include "Q1Way.h"
#include "SysTypes.h"

namespace NodeBase
{
   class CfgParm;
   class CfgTuple;
}

//------------------------------------------------------------------------------

namespace NodeBase
{
//  Global registry for configuration parameters.
//
class CfgParmRegistry : public Protected
{
   friend class Singleton< CfgParmRegistry >;
public:
   //  Deleted to prohibit copying.
   //
   CfgParmRegistry(const CfgParmRegistry& that) = delete;
   CfgParmRegistry& operator=(const CfgParmRegistry& that) = delete;

   //  Adds TUPLE to the registry.
   //
   bool BindTuple(CfgTuple& tuple);

   //  Searches the registry and returns a tuple that matches KEY.
   //  Returns nullptr if no such tuple exists.
   //
   CfgTuple* FindTuple(const std::string& key) const;

   //  Removes TUPLE from the registry.
   //
   void UnbindTuple(CfgTuple& tuple);

   //  Adds PARM to the registry and sets its value from the tuple
   //  that is associated with it.
   //
   bool BindParm(CfgParm& parm);

   //  Searches the registry and returns a configuration parameter
   //  that matches KEY.  Returns nullptr if no such parameter exists.
   //
   CfgParm* FindParm(const std::string& key) const;

   //  Removes PARM from the registry.
   //
   void UnbindParm(CfgParm& parm);

   //  Searches the registry for a configuration parameter that
   //  matches KEY.  If such a parameter exists, updates VALUE to
   //  the string associated with the parameter's current value.
   //
   bool GetValue(const std::string& key, std::string& value) const;

   //  Displays each parameter in the registry, along with its value.
   //
   void ListParms(std::ostream& stream, const std::string& prefix) const;

   //  Returns the arguments that were passed to main().
   //
   const std::vector< stringPtr >& GetMainArgs() const { return *mainArgs_; }

   //  Adds the next argument that was passed to main().
   //
   void AddMainArg(const std::string& arg);

   //  Overridden for restarts.
   //
   void Startup(RestartLevel level) override;

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
   CfgParmRegistry();

   //  Private because this singleton is not subclassed.
   //
   ~CfgParmRegistry();

   //  Reads configuration tuples (key-value pairs) from ConfigFileName
   //  during system initialization.  Creates a CfgTuple instance for
   //  each valid tuple and adds it to the registry.
   //
   void LoadTuples();

   //  Called by LoadTuples to read the next tuple from ConfigFileName.
   //  Returns true if another valid tuple exists; updates KEY and VALUE
   //  accordingly.  Returns false on EOF.  Calls BadLine to log invalid
   //  entries, but continues to look for tuples.
   //
   bool LoadNextTuple(std::string& key, std::string& value);

   //  Called by LoadNextTuple to flag invalid entries in ConfigFileName.
   //  ID identifies the problem, and INPUT is the invalid entry.
   //
   void BadLine(LogId id, const std::string& input) const;

   //  Type for a dynamically allocated vector that holds dynamically
   //  allocated strings.
   //
   typedef std::unique_ptr< std::vector < stringPtr >> stringPtrVectorPtr;

   //  The arguments to main().
   //
   stringPtrVectorPtr mainArgs_;  //r

   //  The file from which tuples are read during system initialization.
   //
   stringPtr configFileName_;  //r

   //  The tuples (key-value pairs) in the registry.  They are kept
   //  in a queue that is sorted in alphabetical order, by key.
   //
   Q1Way< CfgTuple > tupleq_;

   //  The configuration parameters in the registry.  They are kept
   //  in a queue that is sorted in alphabetical order.
   //
   Q1Way< CfgParm > parmq_;

   //  A handle for reading ConfigFileName.
   //
   istreamPtr stream_;  //r

   //  The current line number in ConfigFileName.
   //
   size_t currLine_;

   //> Used to calculate configFileName_, the name of the file that contains
   //  this node's configuration parameters.  It is created by modifying the
   //  first argument to main(), which is the path to our executable, as
   //  follows:
   //  o find the last occurrence of BackFromExePath and erase what *follows*
   //    it (that is, retain BackFromExePath as a "suffix"), and then
   //  o append AppendToExePath.
   //
   static fixed_string BackFromExePath;
   static fixed_string AppendToExePath;
};
}
#endif
