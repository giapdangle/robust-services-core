//==============================================================================
//
//  CxxDirective.cpp
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#include "CxxDirective.h"
#include <bitset>
#include <iosfwd>
#include <ostream>
#include <vector>
#include "CodeFile.h"
#include "CodeTypes.h"
#include "CxxArea.h"
#include "CxxExecute.h"
#include "CxxRoot.h"
#include "CxxSymbols.h"
#include "CxxToken.h"
#include "Debug.h"
#include "Library.h"
#include "Registry.h"
#include "Singleton.h"
#include "SysTypes.h"

using std::ostream;
using std::string;

//------------------------------------------------------------------------------

namespace CodeTools
{
void AlignLeft(ostream& stream, const string& prefix)
{
   //  If PREFIX is more than one indentation, indent one level less.
   //
   auto size = prefix.size();

   if(size < Indent_Size)
      stream << prefix;
   else
      stream << prefix.substr(Indent_Size);
}

//------------------------------------------------------------------------------

fn_name Conditional_ctor = "Conditional.ctor";

Conditional::Conditional()
{
   Debug::ft(Conditional_ctor);
}

//------------------------------------------------------------------------------

void Conditional::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   condition_->Print(stream);
   stream << CRLF;
   OptionalCode::Display(stream, prefix, options);
}

//------------------------------------------------------------------------------

fn_name Conditional_EnterScope = "Conditional.EnterScope";

bool Conditional::EnterScope()
{
   Debug::ft(Conditional_EnterScope);

   //c The expression that follows an #if or #elif is not currently evaluated.
   //  This function returns false, so the code that follows the directive will
   //  be ignored.  To support #if and #elif, the expression would have to be
   //  evaluated so that this function could return true or false as required.

   condition_->EnterBlock();
   auto result = Context::PopArg(true);
   result.CheckIfBool();

   return false;
}

//------------------------------------------------------------------------------

fn_name Conditional_GetUsages = "Conditional.GetUsages";

void Conditional::GetUsages(const CodeFile& file, CxxUsageSets& symbols) const
{
   Debug::ft(Conditional_GetUsages);

   condition_->GetUsages(file, symbols);
}

//==============================================================================

fn_name CxxDirective_ctor = "CxxDirective.ctor";

CxxDirective::CxxDirective()
{
   Debug::ft(CxxDirective_ctor);
}

//==============================================================================

fn_name Define_ctor1 = "Define.ctor";

Define::Define(string& name) : Macro(name),
   rhs_(nullptr),
   value_(nullptr),
   defined_(false)
{
   Debug::ft(Define_ctor1);
}

//------------------------------------------------------------------------------

fn_name Define_ctor2 = "Define.ctor";

Define::Define(string& name, ExprPtr& rhs) : Macro(name),
   rhs_(std::move(rhs)),
   value_(nullptr),
   defined_(true)
{
   Debug::ft(Define_ctor2);
}

//------------------------------------------------------------------------------

fn_name Define_dtor = "Define.dtor";

Define::~Define()
{
   Debug::ft(Define_dtor);
}

//------------------------------------------------------------------------------

CxxToken* Define::AutoType() const
{
   return value_;
}

//------------------------------------------------------------------------------

void Define::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   AlignLeft(stream, prefix);
   stream << HASH_DEFINE_STR << SPACE << *Name();

   if(rhs_ != nullptr)
   {
      stream << SPACE;
      rhs_->Print(stream);
   }

   if(!options.test(DispCode))
   {
      stream << " // r=" << refs_ << SPACE;
      if(!defined_) stream << "[not defined]" << SPACE;
      if(!options.test(DispFQ)) DisplayFiles(stream);
   }

   stream << CRLF;
}

//------------------------------------------------------------------------------

fn_name Define_EnterScope = "Define.EnterScope";

bool Define::EnterScope()
{
   Debug::ft(Define_EnterScope);

   //  If the macro is not yet defined, wait for its definition.
   //
   if(!defined_) return true;

   Context::File()->InsertMacro(this);
   Context::SetPos(GetDeclPos());
   if(!AtFileScope()) Log(DefineNotAtFileScope);

   if(rhs_ != nullptr)
   {
      rhs_->EnterBlock();
      auto result = Context::PopArg(true);
      value_ = result.item;
   }

   return true;
}

//------------------------------------------------------------------------------

fn_name Define_SetExpr = "Define.SetExpr";

void Define::SetExpr(ExprPtr& rhs)
{
   Debug::ft(Define_SetExpr);

   //  Now that the macro has been defined, EnterScope can be invoked.
   //
   rhs_ = std::move(rhs);
   defined_ = true;
   EnterScope();
}

//------------------------------------------------------------------------------

void Define::Shrink()
{
   Macro::Shrink();
   ShrinkExpression(rhs_);
}

//==============================================================================

fn_name Elif_ctor = "Elif.ctor";

Elif::Elif()
{
   Debug::ft(Elif_ctor);

   CxxStats::Incr(CxxStats::ELIF_DIRECTIVE);
}

//------------------------------------------------------------------------------

void Elif::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   AlignLeft(stream, prefix);
   stream << HASH_ELIF_STR << SPACE;
   Conditional::Display(stream, prefix, options);
}

//------------------------------------------------------------------------------

fn_name Elif_EnterScope = "Elif.EnterScope";

bool Elif::EnterScope()
{
   Debug::ft(Elif_EnterScope);

   //  Compile the code that follows the #elif if its #if has not yet compiled
   //  any code and the condition following the #elif evaluates to true.
   //
   Context::SetPos(GetDeclPos());
   auto iff = Context::Optional();
   if(iff->HasCompiledCode()) return false;
   if(!Conditional::EnterScope()) return false;
   SetCompile();
   return true;
}

//==============================================================================

fn_name Else_ctor = "Else.ctor";

Else::Else()
{
   Debug::ft(Else_ctor);

   CxxStats::Incr(CxxStats::ELSE_DIRECTIVE);
}

//------------------------------------------------------------------------------

void Else::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   AlignLeft(stream, prefix);
   stream << HASH_ELSE_STR << CRLF;
   OptionalCode::Display(stream, prefix, options);
}

//------------------------------------------------------------------------------

fn_name Else_EnterScope = "Else.EnterScope";

bool Else::EnterScope()
{
   Debug::ft(Else_EnterScope);

   //  Compile the code that follows the #else if its #if/#ifdef/#ifndef has
   //  not yet compiled any code.
   //
   Context::SetPos(GetDeclPos());
   auto ifx = Context::Optional();
   return !ifx->HasCompiledCode();
}

//==============================================================================

fn_name Endif_ctor = "Endif.ctor";

Endif::Endif()
{
   Debug::ft(Endif_ctor);

   CxxStats::Incr(CxxStats::ENDIF_DIRECTIVE);
}

//------------------------------------------------------------------------------

void Endif::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   AlignLeft(stream, prefix);
   stream << HASH_ENDIF_STR << CRLF;
}

//==============================================================================

fn_name Error_ctor = "Error.ctor";

Error::Error(string& text) : StringDirective(text)
{
   Debug::ft(Error_ctor);

   CxxStats::Incr(CxxStats::ERROR_DIRECTIVE);
}

//------------------------------------------------------------------------------

void Error::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   AlignLeft(stream, prefix);
   stream << HASH_ERROR_STR << SPACE << GetText() << CRLF;
}

//------------------------------------------------------------------------------

fn_name Error_EnterScope = "Error.EnterScope";

bool Error::EnterScope()
{
   Debug::ft(Error_EnterScope);

   Context::SetPos(GetDeclPos());
   Context::SwErr(Error_EnterScope, GetText(), 0);
   return true;
}

//==============================================================================

fn_name Existential_ctor = "Existential.ctor";

Existential::Existential(string& name) :
   else_(nullptr)
{
   Debug::ft(Existential_ctor);

   name_ = MacroNamePtr(new MacroName(name));
}

//------------------------------------------------------------------------------

fn_name Existential_AddElse = "Existential.AddElse";

bool Existential::AddElse(const Else* e)
{
   Debug::ft(Existential_AddElse);

   if(else_ != nullptr) return false;
   else_ = e;
   return true;
}

//------------------------------------------------------------------------------

void Existential::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   stream << *Name() << CRLF;
   OptionalCode::Display(stream, prefix, options);
}

//------------------------------------------------------------------------------

fn_name Existential_GetUsages = "Existential.GetUsages";

void Existential::GetUsages(const CodeFile& file, CxxUsageSets& symbols) const
{
   Debug::ft(Existential_GetUsages);

   auto ref = name_->Referent();
   if(ref != nullptr) symbols.AddDirect(ref);
}

//------------------------------------------------------------------------------

void Existential::Shrink()
{
   name_->Shrink();
}

//==============================================================================

fn_name Ifdef_ctor = "Ifdef.ctor";

Ifdef::Ifdef(string& symbol) : Existential(symbol)
{
   Debug::ft(Ifdef_ctor);

   CxxStats::Incr(CxxStats::IFDEF_DIRECTIVE);
}

//------------------------------------------------------------------------------

void Ifdef::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   AlignLeft(stream, prefix);
   stream << HASH_IFDEF_STR << SPACE;
   Existential::Display(stream, prefix, options);
}

//------------------------------------------------------------------------------

fn_name Ifdef_EnterScope = "Ifdef.EnterScope";

bool Ifdef::EnterScope()
{
   Debug::ft(Ifdef_EnterScope);

   //  Compile the code that follows the #ifdef if the symbol that follows
   //  it has been defined.
   //
   Context::SetPos(GetDeclPos());
   Context::PushOptional(this);
   if(!Existential::SymbolDefined()) return false;
   SetCompile();
   return true;
}

//==============================================================================

fn_name Iff_ctor = "Iff.ctor";

Iff::Iff() :
   else_(nullptr)
{
   Debug::ft(Iff_ctor);

   CxxStats::Incr(CxxStats::IF_DIRECTIVE);
}

//------------------------------------------------------------------------------

fn_name Iff_AddElif = "Iff.AddElif";

bool Iff::AddElif(Elif* e)
{
   Debug::ft(Iff_AddElif);

   if(else_ != nullptr) return false;
   elifs_.push_back(e);
   return true;
}

//------------------------------------------------------------------------------

fn_name Iff_AddElse = "Iff.AddElse";

bool Iff::AddElse(const Else* e)
{
   Debug::ft(Iff_AddElse);

   if(else_ != nullptr) return false;
   else_ = e;
   return true;
}

//------------------------------------------------------------------------------

void Iff::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   AlignLeft(stream, prefix);
   stream << HASH_IF_STR << SPACE;
   Conditional::Display(stream, prefix, options);
}

//------------------------------------------------------------------------------

fn_name Iff_EnterScope = "Iff.EnterScope";

bool Iff::EnterScope()
{
   Debug::ft(Iff_EnterScope);

   //  Compile the code that follows the #if if the condition that follows
   //  evalutes to true.
   //
   Context::SetPos(GetDeclPos());
   Context::PushOptional(this);
   if(!Conditional::EnterScope()) return false;
   SetCompile();
   return true;
}

//------------------------------------------------------------------------------

fn_name Iff_HasCompiledCode = "Iff.HasCompiledCode";

bool Iff::HasCompiledCode() const
{
   Debug::ft(Iff_HasCompiledCode);

   if(HasCompiledCode()) return true;

   for(auto e = elifs_.cbegin(); e != elifs_.cend(); ++e)
   {
      if((*e)->HasCompiledCode()) return true;
   }

   return false;
}

//------------------------------------------------------------------------------

void Iff::Shrink()
{
   Conditional::Shrink();
   elifs_.shrink_to_fit();
   CxxStats::Vectors(CxxStats::IF_DIRECTIVE, elifs_.capacity());
}

//==============================================================================

fn_name Ifndef_ctor = "Ifndef.ctor";

Ifndef::Ifndef(string& symbol) : Existential(symbol)
{
   Debug::ft(Ifndef_ctor);

   CxxStats::Incr(CxxStats::IFNDEF_DIRECTIVE);
}

//------------------------------------------------------------------------------

void Ifndef::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   AlignLeft(stream, prefix);
   stream << HASH_IFNDEF_STR << SPACE;
   Existential::Display(stream, prefix, options);
}

//------------------------------------------------------------------------------

fn_name Ifndef_EnterScope = "Ifndef.EnterScope";

bool Ifndef::EnterScope()
{
   Debug::ft(Ifndef_EnterScope);

   //  Compile the code that follows the #ifdef if the symbol that follows
   //  it has not been defined.
   //
   Context::SetPos(GetDeclPos());
   Context::PushOptional(this);
   if(Existential::SymbolDefined()) return false;
   SetCompile();
   return true;
}

//==============================================================================

fn_name Include_ctor = "Include.ctor";

Include::Include(string& name, bool angle) : SymbolDirective(name),
   angle_(angle)
{
   Debug::ft(Include_ctor);

   CxxStats::Incr(CxxStats::INCLUDE_DIRECTIVE);
}

//------------------------------------------------------------------------------

void Include::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   AlignLeft(stream, prefix);
   stream << HASH_INCLUDE_STR << SPACE;
   stream << (angle_ ? '<' : QUOTE);
   stream << *Name();
   stream << (angle_ ? '>' : QUOTE);
   stream << CRLF;
}

//------------------------------------------------------------------------------

fn_name Include_SetScope = "Include.SetScope";

void Include::SetScope(CxxScope* scope) const
{
   Debug::ft(Include_SetScope);

   if(scope == nullptr) return;
   if(scope->Type() == Cxx::Namespace) return;
   Log(IncludeNotAtFileScope);
}

//------------------------------------------------------------------------------

void Include::Shrink()
{
   SymbolDirective::Shrink();
   CxxStats::Strings(CxxStats::INCLUDE_DIRECTIVE, Name()->capacity());
}

//==============================================================================

fn_name Line_ctor = "Line.ctor";

Line::Line(string& text) : StringDirective(text)
{
   Debug::ft(Line_ctor);

   CxxStats::Incr(CxxStats::LINE_DIRECTIVE);
}

//------------------------------------------------------------------------------

void Line::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   AlignLeft(stream, prefix);
   stream << HASH_LINE_STR << SPACE << GetText() << CRLF;
}

//==============================================================================

fn_name Macro_ctor = "Macro.ctor";

Macro::Macro(string& name) :
   refs_(0)
{
   Debug::ft(Macro_ctor);

   std::swap(name_, name);
   SetScope(Singleton< CxxRoot >::Instance()->GlobalNamespace());
   Singleton< CxxSymbols >::Instance()->InsertMacro(this);
   CxxStats::Incr(CxxStats::DEFINE_DIRECTIVE);
}

//------------------------------------------------------------------------------

fn_name Macro_dtor = "Macro.dtor";

Macro::~Macro()
{
   Debug::ft(Macro_dtor);

   Singleton< CxxSymbols >::Instance()->EraseMacro(this);
   CxxStats::Decr(CxxStats::DEFINE_DIRECTIVE);
}

//------------------------------------------------------------------------------

void Macro::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   AlignLeft(stream, prefix);
   stream << HASH_DEFINE_STR << SPACE;
   stream << *Name();

   if(!options.test(DispCode))
   {
      stream << " // r=" << refs_ << SPACE;
      stream << "built-in";
   }

   stream << CRLF;
}

//------------------------------------------------------------------------------

fn_name Macro_Empty = "Macro.Empty";

bool Macro::Empty() const
{
   Debug::ft(Macro_Empty);

   return (GetValue() == nullptr);
}

//------------------------------------------------------------------------------

fn_name Macro_GetNumeric = "Macro.GetNumeric";

Numeric Macro::GetNumeric() const
{
   Debug::ft(Macro_GetNumeric);

   auto ref = Referent();
   if(ref != nullptr) return ref->GetNumeric();
   return Numeric::Nil;
}

//------------------------------------------------------------------------------

fn_name Macro_GetValue = "Macro.GetValue";

CxxToken* Macro::GetValue() const
{
   Debug::ft(Macro_GetValue);

   //  This is a pure virtual function.
   //
   Debug::SwErr(Macro_GetValue, 0, 0);
   return nullptr;
}

//------------------------------------------------------------------------------

fn_name Macro_SetExpr = "Macro.SetExpr";

void Macro::SetExpr(ExprPtr& rhs)
{
   Debug::ft(Macro_SetExpr);

   //  This shouldn't be invoked on a built-in macro.
   //
   Debug::SwErr(Macro_SetExpr, 0, 0);
}

//------------------------------------------------------------------------------

void Macro::Shrink()
{
   name_.shrink_to_fit();
   CxxStats::Strings(CxxStats::DEFINE_DIRECTIVE, Name()->capacity());
}

//------------------------------------------------------------------------------

fn_name Macro_TypeString = "Macro.TypeString";

string Macro::TypeString(bool arg) const
{
   Debug::ft(Macro_TypeString);

   auto value = GetValue();
   if(value != nullptr) return value->TypeString(arg);
   return EMPTY_STR;
}

//==============================================================================

fn_name MacroName_ctor = "MacroName.ctor";

MacroName::MacroName(string& name) :
   ref_(nullptr),
   defined_(false)
{
   Debug::ft(MacroName_ctor);

   std::swap(name_, name);
   CxxStats::Incr(CxxStats::MACRO_NAME);
}

//------------------------------------------------------------------------------

fn_name MacroName_dtor = "MacroName.dtor";

MacroName::~MacroName()
{
   Debug::ft(MacroName_dtor);

   CxxStats::Decr(CxxStats::QUAL_NAME);
}

//------------------------------------------------------------------------------

fn_name MacroName_EnterBlock = "MacroName.EnterBlock";

void MacroName::EnterBlock()
{
   Debug::ft(MacroName_EnterBlock);

   Context::PushArg(StackArg(Referent(), 0));
}

//------------------------------------------------------------------------------

fn_name MacroName_GetUsages = "MacroName.GetUsages";

void MacroName::GetUsages(const CodeFile& file, CxxUsageSets& symbols) const
{
   Debug::ft(MacroName_GetUsages);

   //  Add our referent as a direct usage.
   //
   if(ref_ != nullptr) symbols.AddDirect(ref_);
}

//------------------------------------------------------------------------------

void MacroName::Print(ostream& stream) const
{
   stream << name_;
}

//------------------------------------------------------------------------------

fn_name MacroName_Referent = "MacroName.Referent";

CxxNamed* MacroName::Referent() const
{
   Debug::ft(MacroName_Referent);

   //  This is invoked to find a referent in a preprocessor directive.
   //
   if(ref_ != nullptr) return ref_;

   //  Look for the macro name.  If it is visible, it has not necessarily been
   //  defined: it could have been used in a file that is visible to this one,
   //  but only in a conditional compilation directive that caused it to be
   //  added to the symbol table, which is done at the bottom of this function.
   //
   auto syms = Singleton< CxxSymbols >::Instance();
   auto file = Context::File();
   auto scope = Singleton< CxxRoot >::Instance()->GlobalNamespace();
   SymbolView view;
   ref_ = syms->FindSymbol(file, scope, name_, MACRO_MASK, &view);

   if(ref_ != nullptr)
   {
      auto macro = static_cast< Macro* >(ref_);
      defined_ = macro->IsDefined();
      macro->WasRead();
      return ref_;
   }

   //  Look for the macro name again, even if it has been defined in a file
   //  that is not visible to this one.
   //
   ref_ = syms->FindMacro(name_);

   if(ref_ != nullptr)
   {
      auto macro = static_cast< Macro* >(ref_);
      macro->WasRead();
      return ref_;
   }

   //  This is the first appearance of the macro name, so create a placeholder
   //  for it.
   //
   auto name = name_;
   auto macro = MacroPtr(new Define(name));
   ref_ = macro.get();
   Singleton< CxxRoot >::Instance()->AddMacro(macro);
   ref_->WasRead();
   return ref_;
}

//------------------------------------------------------------------------------

void MacroName::Shrink()
{
   name_.shrink_to_fit();
   CxxStats::Strings(CxxStats::MACRO_NAME, name_.capacity());
}

//------------------------------------------------------------------------------

fn_name MacroName_TypeString = "MacroName.TypeString";

string MacroName::TypeString(bool arg) const
{
   auto ref = Referent();

   if(ref != nullptr)
   {
      return ref->TypeString(arg);
   }

   auto expl = "Failed to find referent for " + name_;
   Context::SwErr(MacroName_TypeString, expl, 0);
   return ERROR_STR;
}

//------------------------------------------------------------------------------

fn_name MacroName_WasDefined = "MacroName.WasDefined";

bool MacroName::WasDefined() const
{
   Debug::ft(MacroName_WasDefined);

   //  Make sure that the referent has been searched for.
   //
   Referent();
   return defined_;
}

//==============================================================================

fn_name Optional_ctor = "Optional.ctor";

Optional::Optional()
{
   Debug::ft(Optional_ctor);
}

//==============================================================================

fn_name OptionalCode_ctor = "OptionalCode.ctor";

OptionalCode::OptionalCode() :
   begin_(string::npos),
   end_(0),
   compile_(false)
{
   Debug::ft(OptionalCode_ctor);
}

//------------------------------------------------------------------------------

void OptionalCode::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   if(compile_) return;

   if(begin_ == string::npos) return;

   auto file = Singleton< Library >::Instance()->Files().At(GetDeclFid());

   if(file == nullptr)
   {
      stream << "ERROR: FILE NOT FOUND" << CRLF;
      return;
   }

   auto code = file->GetCode();

   if(code->size() < end_)
   {
      stream << "ERROR: CODE NOT FOUND" << CRLF;
      return;
   }

   stream << prefix;

   for(auto i = begin_; i < end_; ++i)
   {
      stream << code->at(i);
      if(code->at(i) == CRLF) stream << prefix;
   }

   stream << CRLF;
}

//==============================================================================

fn_name Pragma_ctor = "Pragma.ctor";

Pragma::Pragma(string& text) : StringDirective(text)
{
   Debug::ft(Pragma_ctor);

   CxxStats::Incr(CxxStats::PRAGMA_DIRECTIVE);
}

//------------------------------------------------------------------------------

void Pragma::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   AlignLeft(stream, prefix);
   stream << HASH_PRAGMA_STR << SPACE << GetText() << CRLF;
}

//==============================================================================

fn_name StringDirective_ctor = "StringDirective.ctor";

StringDirective::StringDirective(string& text)
{
   Debug::ft(StringDirective_ctor);

   std::swap(text_, text);
}

//------------------------------------------------------------------------------

void StringDirective::Shrink()
{
   text_.shrink_to_fit();
}

//==============================================================================

fn_name SymbolDirective_ctor = "SymbolDirective.ctor";

SymbolDirective::SymbolDirective(string& name)
{
   Debug::ft(SymbolDirective_ctor);

   std::swap(name_, name);
}

//------------------------------------------------------------------------------

void SymbolDirective::Shrink()
{
   name_.shrink_to_fit();
}

//==============================================================================

fn_name Undef_ctor = "Undef.ctor";

Undef::Undef(string& name) : SymbolDirective(name)
{
   Debug::ft(Undef_ctor);

   CxxStats::Incr(CxxStats::UNDEF_DIRECTIVE);
}

//------------------------------------------------------------------------------

void Undef::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   AlignLeft(stream, prefix);
   stream << HASH_UNDEF_STR << SPACE;
   stream << *Name();
   stream << CRLF;
}

//------------------------------------------------------------------------------

void Undef::Shrink()
{
   SymbolDirective::Shrink();
   CxxStats::Strings(CxxStats::UNDEF_DIRECTIVE, Name()->capacity());
}
}