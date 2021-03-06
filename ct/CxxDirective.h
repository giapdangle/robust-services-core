//==============================================================================
//
//  CxxDirective.h
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
#ifndef CXXDIRECTIVE_H_INCLUDED
#define CXXDIRECTIVE_H_INCLUDED

#include "CxxNamed.h"
#include "CxxScoped.h"
#include <cstddef>
#include <string>
#include <utility>
#include "Cxx.h"
#include "CxxFwd.h"
#include "CxxToken.h"

//------------------------------------------------------------------------------

namespace CodeTools
{
//  Base class for preprocessor directives (except for #define).
//
class CxxDirective : public CxxNamed
{
public:
   //  Virtual to allow subclassing.
   //
   virtual ~CxxDirective() = default;

   //  Overridden to prevent a log when a directive appears inside a function.
   //
   void EnterBlock() override { }

   //  Overridden to indicate that directives cannot be displayed inline.
   //
   bool InLine() const override { return false; }
protected:
   //  Protected because this class is virtual.
   //
   CxxDirective();
};

//------------------------------------------------------------------------------
//
//  Base class for #include and #undef.
//
class SymbolDirective : public CxxDirective
{
public:
   //  Virtual to allow subclassing.
   //
   virtual ~SymbolDirective() = default;

   //  Overridden to return the symbol's name.
   //
   const std::string* Name() const override { return &name_; }

   //  Overridden to shrink the item's name.
   //
   void Shrink() override;
protected:
   //  Creates a directive that defines or references NAME.  Protected because
   //  this class is virtual.
   //
   explicit SymbolDirective(std::string& name);
private:
   //  The symbol that was defined.
   //
   std::string name_;
};

//------------------------------------------------------------------------------
//
//  An #include directive.
//
class Include : public SymbolDirective
{
public:
   //  Creates an #include directive for the file identified by NAME.  ANGLE
   //  is set if the name appeared in angle brackets.
   //
   Include(std::string& name, bool angle);

   //  Not subclassed.
   //
   ~Include() { CxxStats::Decr(CxxStats::INCLUDE_DIRECTIVE); }

   //  Returns the file associated with the directive.
   //
   CodeFile* FindFile() const;

   //  Overridden to display the directive.
   //
   void Display(std::ostream& stream,
      const std::string& prefix, const NodeBase::Flags& options) const override;

   //  Overridden to report the filename's length.
   //
   void Shrink() override;
private:
   //  Set if the filename appeared in angle brackets.
   //
   const bool angle_;
};

//------------------------------------------------------------------------------
//
//  An #undef directive.
//
class Undef : public SymbolDirective
{
public:
   //  Creates a #undef directive for the symbol identified by NAME.
   //
   explicit Undef(std::string& name);

   //  Not subclassed.
   //
   ~Undef() { CxxStats::Decr(CxxStats::UNDEF_DIRECTIVE); }

   //  Overridden to display the directive.
   //
   void Display(std::ostream& stream,
      const std::string& prefix, const NodeBase::Flags& options) const override;

   //  Overridden to report the symbol's length.
   //
   void Shrink() override;
};

//------------------------------------------------------------------------------
//
//  A name that appears in a preprocessor directive.
//
class MacroName : public CxxNamed
{
public:
   //  Creates a name that begins with NAME.
   //
   explicit MacroName(std::string& name);

   //  Not subclassed.
   //
   ~MacroName();

   //  Returns true if the name was defined when it was encountered.
   //
   bool WasDefined() const;

   //  Overridden to find the referent and push it onto the argument stack.
   //
   void EnterBlock() override;

   //  Overridden to return the global namespace.
   //
   CxxScope* GetScope() const override;

   //  Overridden to update SYMBOLS with the name's type usage.
   //
   void GetUsages(const CodeFile& file, CxxUsageSets& symbols) const override;

   //  Overridden to return the macro's name.
   //
   const std::string* Name() const override { return &name_; }

   //  Overridden to display the name, including any template arguments.
   //
   void Print
      (std::ostream& stream, const NodeBase::Flags& options) const override;

   //  Overridden to return the macro's name.
   //
   std::string QualifiedName
      (bool scopes, bool templates) const override { return name_; }

   //  Overridden to return what the name refers to.
   //
   CxxScoped* Referent() const override;

   //  Overridden to shrink containers.
   //
   void Shrink() override;

   //  Overridden to return the referent's full root type.
   //
   std::string TypeString(bool arg) const override;
private:
   //  The macro's name.
   //
   std::string name_;

   //  What the name refers to.
   //
   mutable CxxScoped* ref_;

   //  Whether the name was defined when it was encountered.
   //
   mutable bool defined_;
};

//------------------------------------------------------------------------------
//
//  Base class for #define and built-in macros.
//
class Macro : public CxxScoped
{
public:
   //  Virtual to allow subclassing.
   //
   virtual ~Macro();

   //  Returns true if the macro maps to an empty string.
   //
   bool Empty() const;

   //  Returns the macro's value.  Returns nullptr if the macro name has
   //  been used but has not yet been defined.
   //
   virtual CxxToken* GetValue() const = 0;

   //  Returns true if the macro has been defined.  Because this class is
   //  only used for built-in macros, it always returns true.
   //
   virtual bool IsDefined() const { return true; }

   //  Sets RHS as the macro's definition when the macro name appears in a
   //  #define after its name was already used.  If invoked on this class,
   //  which is only used for built-in macros, a log is output.
   //
   virtual void SetExpr(ExprPtr& rhs);

   //  Overridden to display the macro.
   //
   void Display(std::ostream& stream,
      const std::string& prefix, const NodeBase::Flags& options) const override;

   //  Overridden to invoke GetNumeric on the referent, if found.
   //
   Numeric GetNumeric() const override;

   //  Overridden to indicate that a #define cannot be displayed inline.
   //
   bool InLine() const override { return false; }

   //  Overridden to determine if the macro is unused.
   //
   bool IsUnused() const override { return (refs_ == 0); }

   //  Overridden to return the macro's name.
   //
   const std::string* Name() const override { return &name_; }

   //  Overridden for when NAME refers to a macro set for the compiler.
   //
   bool NameRefersToItem(const std::string& name, const CxxScope* scope,
      const CodeFile* file, SymbolView* view) const override;

   //  Overridden to record usage of the macro.
   //
   void RecordUsage() const override { AddUsage(); }

   //  Overridden to return the underlying type.
   //
   CxxToken* RootType() const override { return GetValue(); }

   //  Overridden to shrink the item's name.
   //
   void Shrink() override;

   //  Overridden to reveal that this is a macro.
   //
   Cxx::ItemType Type() const override { return Cxx::Macro; }

   //  Overridden to invoke TypeString on the referent, if found.
   //
   std::string TypeString(bool arg) const override;

   //  Overridden to count references to the macro.
   //
   bool WasRead() override { ++refs_; return true; }
protected:
   //  Creates a macro for the symbol identified by NAME.  Protected
   //  because this class is virtual.
   //
   explicit Macro(const std::string& name);

   //  How many times the macro was referenced.
   //
   size_t refs_ : 16;
private:
   //  The macro's name.
   //
   std::string name_;
};

//------------------------------------------------------------------------------
//
//  A #define directive.
//
class Define : public Macro
{
public:
   //  Creates a #define directive for the macro identified by NAME.  This
   //  constructor is used when NAME has not yet appeared in a #define when
   //  it is used in a conditional compilation directive.  The most common
   //  example is an #include guard: #ifndef <name> #define <name>: a Define
   //  for <name> is created while processing the #ifndef, even though <name>
   //  is not defined until the next statement.
   //
   explicit Define(const std::string& name);

   //  Creates a #define directive for the macro identified by NAME, which
   //  has the value associated with RHS.  This constructor is used when NAME
   //  appears in a #define, and must be used even if RHS is empty.
   //
   Define(const std::string& name, ExprPtr& rhs);

   //  Not subclassed.
   //
   ~Define();

   //  Overridden to return the type associated with the macro.
   //
   CxxToken* AutoType() const override;

   //  Overridden to return true if the macro name has appeared in a #define.
   //
   bool IsDefined() const override { return defined_; }

   //  Overridden to set RHS as the macro's definition when the macro name
   //  appears in a #define after its name was already used.
   //
   void SetExpr(ExprPtr& rhs) override;

   //  Returns the macro's underlying value.
   //
   CxxToken* GetValue() const override { return value_; }

   //  Overridden to display the directive.
   //
   void Display(std::ostream& stream,
      const std::string& prefix, const NodeBase::Flags& options) const override;

   //  Overridden to log a #define that is not at file scope.
   //
   bool EnterScope() override;

   //  Overridden to shrink containers.
   //
   void Shrink() override;
private:
   //  The expression, if any, that assigns a value to the macro.
   //
   ExprPtr rhs_;

   //  The symbol's underlying value.
   //
   CxxToken* value_;

   //  Set if the macro name has appeared in a #define.
   //
   bool defined_;
};

//------------------------------------------------------------------------------
//
//  Base class for #if, #ifdef, #ifndef, #elif, #else, and #endif.
//
class Optional : public CxxDirective
{
public:
   //  Virtual to allow subclassing.
   //
   virtual ~Optional() = default;
protected:
   //  Protected because this class is virtual.
   //
   explicit Optional();
};

//------------------------------------------------------------------------------
//
//  Source code that follows an #if, #ifdef, #ifndef, #elif, or #else.
//
class OptionalCode : public Optional
{
public:
   //  Virtual to allow subclassing.
   //
   virtual ~OptionalCode() = default;

   //  Invoked when it is determined that the code following the directive
   //  should not be compiled.
   //
   void SetSkipped(size_t begin, size_t end) { begin_ = begin; end_ = end; }

   //  Adds an #elif to the directive.
   //
   virtual bool AddElif(Elif* e) { return false; }

   //  Adds an #else to the directive.
   //
   virtual bool AddElse(const Else* e) { return false; }

   //  Overridden to display source code if it was not compiled.
   //
   void Display(std::ostream& stream,
      const std::string& prefix, const NodeBase::Flags& options) const override;

   //  Overridden to return true if compiled code follows this directive.
   //
   virtual bool HasCompiledCode() const { return compile_; }
protected:
   //  Protected because this class is virtual.
   //
   explicit OptionalCode();

   //  Invoked when it is determined that the code following the directive
   //  should be compiled.
   //
   void SetCompile() { compile_ = true; }
private:
   //  Adds a condition to the directive.
   //
   virtual void AddCondition(ExprPtr& c) { }

   //  Where the code that follows the directive begins.
   //
   size_t begin_;

   //  Where the code that follows the directive ends.
   //
   size_t end_;

   //  Set if the code that follows the directive is to be compiled.
   //
   bool compile_;
};

//------------------------------------------------------------------------------
//
//  Base class for #if and #elif.
//
class Conditional : public OptionalCode
{
public:
   //  Virtual to allow subclassing.
   //
   virtual ~Conditional() = default;

   //  Overridden to add a condition to the directive.
   //
   void AddCondition(ExprPtr& c) override { condition_ = std::move(c); }

   //  Overridden to display the condition.
   //
   void Display(std::ostream& stream,
      const std::string& prefix, const NodeBase::Flags& options) const override;

   //  Overridden to return the result of evaluating the condition.
   //
   bool EnterScope() override;

   //  Overridden to include symbols that appear in the condition.
   //
   void GetUsages(const CodeFile& file, CxxUsageSets& symbols) const override;

   //  Overridden to shrink the conditional expression.
   //
   void Shrink() override { condition_->Shrink(); }
protected:
   //  Protected because this class is virtual.
   //
   explicit Conditional();
private:
   //  The condition associated with the directive.
   //
   ExprPtr condition_;
};

//------------------------------------------------------------------------------
//
//  Base class for #ifdef and #ifndef.
//
class Existential : public OptionalCode
{
public:
   //  Virtual to allow subclassing.
   //
   virtual ~Existential() = default;

   //  Overridden to add an #else to the directive.
   //
   bool AddElse(const Else* e) override;

   //  Overridden to display the directive.
   //
   void Display(std::ostream& stream,
      const std::string& prefix, const NodeBase::Flags& options) const override;

   //  Overridden to update SYMBOLS with name_'s referent.
   //
   void GetUsages(const CodeFile& file, CxxUsageSets& symbols) const override;

   //  Overridden to return the symbol's name.
   //
   const std::string* Name() const override { return name_->Name(); }

   //  Overridden to shrink the item's name.
   //
   void Shrink() override;
protected:
   //  MACRO is the symbol whose existence an #ifdef or #ifndef is checking.
   //  Protected because this class is virtual.
   //
   explicit Existential(MacroNamePtr& macro);

   //  Returns true if name_ has been defined.
   //
   bool SymbolDefined() const { return name_->WasDefined(); }
private:
   //  The symbol whose definition the directive is checking.
   //
   MacroNamePtr name_;

   //  Any #else clause associated with the directive.
   //
   const Else* else_;
};

//------------------------------------------------------------------------------
//
//  An #elif directive.
//
class Elif : public Conditional
{
public:
   //  Creates an #elif directive.
   //
   explicit Elif();

   //  Not subclassed.
   //
   ~Elif() { CxxStats::Decr(CxxStats::ELIF_DIRECTIVE); }

   //  Overridden to display the directive.
   //
   void Display(std::ostream& stream,
      const std::string& prefix, const NodeBase::Flags& options) const override;

   //  Overridden to handle the code that follows the #elif.
   //
   bool EnterScope() override;
};

//------------------------------------------------------------------------------
//
//  An #else directive.
//
class Else : public OptionalCode
{
public:
   //  Creates an #else directive.
   //
   explicit Else();

   //  Not subclassed.
   //
   ~Else() { CxxStats::Decr(CxxStats::ELSE_DIRECTIVE); }

   //  Overridden to display the directive.
   //
   void Display(std::ostream& stream,
      const std::string& prefix, const NodeBase::Flags& options) const override;

   //  Overridden to handle the code that follows the #else.
   //
   bool EnterScope() override;
};

//------------------------------------------------------------------------------
//
//  An #endif directive.
//
class Endif : public Optional
{
public:
   //  Creates an #endif directive.
   //
   explicit Endif();

   //  Not subclassed.
   //
   ~Endif() { CxxStats::Decr(CxxStats::ENDIF_DIRECTIVE); }

   //  Overridden to display the directive.
   //
   void Display(std::ostream& stream,
      const std::string& prefix, const NodeBase::Flags& options) const override;
};

//------------------------------------------------------------------------------
//
//  An #ifdef preprocessor directive.
//
class Ifdef : public Existential
{
public:
   //  Creates an #ifdef directive that checks for the existence of MACRO.
   //
   explicit Ifdef(MacroNamePtr& macro);

   //  Not subclassed.
   //
   ~Ifdef() { CxxStats::Decr(CxxStats::IFDEF_DIRECTIVE); }

   //  Overridden to display the directive.
   //
   void Display(std::ostream& stream,
      const std::string& prefix, const NodeBase::Flags& options) const override;

   //  Overridden to handle the code that follows the #ifdef.
   //
   bool EnterScope() override;
};

//------------------------------------------------------------------------------
//
//  An #if directive.
//
class Iff : public Conditional
{
public:
   //  Creates an #if directive.
   //
   Iff();

   //  Not subclassed.
   //
   ~Iff() { CxxStats::Decr(CxxStats::IF_DIRECTIVE); }

   //  Overridden to add an #elif.
   //
   bool AddElif(Elif* e) override;

   //  Overridden to add an #else.
   //
   bool AddElse(const Else* e) override;

   //  Overridden to display the directive.
   //
   void Display(std::ostream& stream,
      const std::string& prefix, const NodeBase::Flags& options) const override;

   //  Overridden to handle the code that follows the #if.
   //
   bool EnterScope() override;

   //  Overridden to return true if compiled code follows this directive
   //  or an #elif.
   //
   bool HasCompiledCode() const override;

   //  Overridden to shrink containers.
   //
   void Shrink() override;
private:
   //  Any #elifs that follow the #if.
   //
   ElifVector elifs_;

   //  Any #else that follows the #if.
   //
   const Else* else_;
};

//------------------------------------------------------------------------------
//
//  An #ifndef directive.
//
class Ifndef : public Existential
{
public:
   //  Creates an #ifndef directive that checks for the existence of MACRO.
   //
   explicit Ifndef(MacroNamePtr& macro);

   //  Not subclassed.
   //
   ~Ifndef() { CxxStats::Decr(CxxStats::IFNDEF_DIRECTIVE); }

   //  Overridden to display the directive.
   //
   void Display(std::ostream& stream,
      const std::string& prefix, const NodeBase::Flags& options) const override;

   //  Overridden to handle the code that follows the #ifndef.
   //
   bool EnterScope() override;
};

//------------------------------------------------------------------------------
//
//  Base class for #pragma, #error, and #line.
//
class StringDirective : public CxxDirective
{
public:
   //  Virtual to allow subclassing.
   //
   virtual ~StringDirective() = default;

   //  Returns the text that follows the directive.
   //
   const std::string& GetText() const { return text_; }

   //  Overridden to shrink the item's string.
   //
   void Shrink() override;
protected:
   //  Creates a directive that is followed by TEXT.  Protected because
   //  this class is virtual.
   //
   explicit StringDirective(std::string& text);
private:
   //  The text that follows the directive.
   //
   std::string text_;
};

//------------------------------------------------------------------------------
//
//  A #pragma directive.
//
class Pragma : public StringDirective
{
public:
   //  Creates a #pragma directive that is followed by TEXT.
   //
   explicit Pragma(std::string& text);

   //  Not subclassed.
   //
   ~Pragma() { CxxStats::Decr(CxxStats::PRAGMA_DIRECTIVE); }

   //  Overridden to display the directive.
   //
   void Display(std::ostream& stream,
      const std::string& prefix, const NodeBase::Flags& options) const override;
};

//------------------------------------------------------------------------------
//
//  An #error directive.
//
class Error : public StringDirective
{
public:
   //  Creates an #error directive that is followed by TEXT.
   //
   explicit Error(std::string& text);

   //  Not subclassed.
   //
   ~Error() { CxxStats::Decr(CxxStats::ERROR_DIRECTIVE); }

   //  Overridden to display the directive.
   //
   void Display(std::ostream& stream,
      const std::string& prefix, const NodeBase::Flags& options) const override;

   //  Overridden to generate a log.
   //
   bool EnterScope() override;
};

//------------------------------------------------------------------------------
//
//  A #line directive.
//
class Line : public StringDirective
{
public:
   //  Creates a #line directive that is followed by TEXT.
   //
   explicit Line(std::string& text);

   //  Not subclassed.
   //
   ~Line() { CxxStats::Decr(CxxStats::LINE_DIRECTIVE); }

   //  Overridden to display the directive.
   //
   void Display(std::ostream& stream,
      const std::string& prefix, const NodeBase::Flags& options) const override;
};
}
#endif
