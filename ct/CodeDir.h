//==============================================================================
//
//  CodeDir.h
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#ifndef CODEDIR_H_INCLUDED
#define CODEDIR_H_INCLUDED

#include "LibraryItem.h"
#include <cstddef>
#include <string>
#include "RegCell.h"
#include "SysTypes.h"

using namespace NodeBase;

//------------------------------------------------------------------------------

namespace CodeTools
{
//  Provides access to a directory that contains source code.
//
class CodeDir : public LibraryItem
{
public:
   //  Creates an instance for PATH, which will be referred to by NAME.
   //
   CodeDir(const std::string& name, const std::string& path);

   //  Not subclassed.
   //
   ~CodeDir();

   //  Finds all of the .h and .cpp files in the directory.  Updates
   //  EXPL to indicate success or failure.  Returns 0 on success.
   //  The implementation is platform specific because C++ does not
   //  provide a way to iterate over the files in a directory.
   //
   word Extract(std::string& expl);

   //  Returns the directory's identifier.
   //
   id_t Did() const { return did_.GetId(); }

   //  Returns the directory's path.
   //
   const std::string& Path() const { return path_; }

   //  Returns true if the directory contains substitute files.
   //
   bool IsSubsDir() const;

   //  Returns the number of .h files in the directory.
   //
   size_t HeaderCount() const;

   //  Returns the number of .cpp files in the directory.
   //
   size_t CppCount() const;

   //  Returns the offset to did_.
   //
   static ptrdiff_t CellDiff();

   //  Overridden to display member variables.
   //
   virtual void Display(std::ostream& stream,
      const std::string& prefix, const Flags& options) const override;
private:
   //  Returns true if NAME is a code file.
   //
   static bool IsCodeFile(const std::string& name);

   //  The directory's identifier in the code base.
   //
   RegCell did_;

   //  The directory's path.
   //
   const std::string path_;
};
}
#endif