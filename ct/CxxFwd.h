//==============================================================================
//
//  CxxFwd.h
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#ifndef CXXFWD_H_INCLUDED
#define CXXFWD_H_INCLUDED

#include <memory>
#include <set>
#include <string>
#include <vector>

//------------------------------------------------------------------------------

namespace CodeTools
{
//  Forward declarations of classes that represent C++ language constructs.
//
class Argument;
class ArraySpec;
class BaseDecl;
class Block;
class Class;
class ClassInst;
class CodeFile;
class CxxArea;
class CxxDirective;
class CxxNamed;
class CxxScope;
class CxxScoped;
class CxxToken;
class Data;
class Define;
class Elif;
class Else;
class Endif;
class Enum;
class Enumerator;
class Error;
class Expression;
class Forward;
class Friend;
class FuncData;
class Function;
class Ifdef;
class Iff;
class Ifndef;
class Include;
class Line;
class Macro;
class MacroName;
class MemberInit;
class Namespace;
class Operation;
class OptionalCode;
class ParseFrame;
class Pragma;
class QualName;
class StackArg;
class TemplateParm;
class TemplateParms;
class Terminal;
class Typedef;
class TypeName;
class TypeSpec;
class Undef;
class Using;

//------------------------------------------------------------------------------
//
//  Types for unique_ptrs that own instances of the above classes.
//
typedef std::unique_ptr< Argument > ArgumentPtr;
typedef std::unique_ptr< ArraySpec > ArraySpecPtr;
typedef std::unique_ptr< BaseDecl > BaseDeclPtr;
typedef std::unique_ptr< Block > BlockPtr;
typedef std::unique_ptr< Class > ClassPtr;
typedef std::unique_ptr< ClassInst > ClassInstPtr;
typedef std::unique_ptr< CxxDirective > DirectivePtr;
typedef std::unique_ptr< CxxToken > TokenPtr;
typedef std::unique_ptr< Data > DataPtr;
typedef std::unique_ptr< Define > DefinePtr;
typedef std::unique_ptr< Elif > ElifPtr;
typedef std::unique_ptr< Else > ElsePtr;
typedef std::unique_ptr< Endif > EndifPtr;
typedef std::unique_ptr< Error > ErrorPtr;
typedef std::unique_ptr< Enum > EnumPtr;
typedef std::unique_ptr< Enumerator > EnumeratorPtr;
typedef std::unique_ptr< Expression > ExprPtr;
typedef std::unique_ptr< Forward > ForwardPtr;
typedef std::unique_ptr< Friend > FriendPtr;
typedef std::unique_ptr< Function > FunctionPtr;
typedef std::unique_ptr< Ifdef > IfdefPtr;
typedef std::unique_ptr< Iff > IffPtr;
typedef std::unique_ptr< Ifndef > IfndefPtr;
typedef std::unique_ptr< Include > IncludePtr;
typedef std::unique_ptr< Line > LinePtr;
typedef std::unique_ptr< Macro > MacroPtr;
typedef std::unique_ptr< MacroName > MacroNamePtr;
typedef std::unique_ptr< MemberInit > MemberInitPtr;
typedef std::unique_ptr< Namespace > NamespacePtr;
typedef std::unique_ptr< ParseFrame > ParseFramePtr;
typedef std::unique_ptr< Pragma > PragmaPtr;
typedef std::unique_ptr< QualName > QualNamePtr;
typedef std::unique_ptr< TemplateParm > TemplateParmPtr;
typedef std::unique_ptr< TemplateParms > TemplateParmsPtr;
typedef std::unique_ptr< Terminal > TerminalPtr;
typedef std::unique_ptr< Typedef > TypedefPtr;
typedef std::unique_ptr< TypeName > TypeNamePtr;
typedef std::unique_ptr< TypeSpec > TypeSpecPtr;
typedef std::unique_ptr< Undef > UndefPtr;
typedef std::unique_ptr< Using > UsingPtr;

//------------------------------------------------------------------------------
//
//  Types for containers of the above classes.
//
typedef std::vector< ArraySpecPtr > ArraySpecPtrVector;
typedef std::vector< ArgumentPtr > ArgumentPtrVector;
typedef std::vector< ClassPtr > ClassPtrVector;
typedef std::vector< ClassInstPtr > ClassInstPtrVector;
typedef std::vector< DataPtr > DataPtrVector;
typedef std::vector< DirectivePtr > DirectivePtrVector;
typedef std::vector< EnumPtr > EnumPtrVector;
typedef std::vector< EnumeratorPtr > EnumeratorPtrVector;
typedef std::vector< ForwardPtr > ForwardPtrVector;
typedef std::vector< FriendPtr > FriendPtrVector;
typedef std::vector< FunctionPtr > FunctionPtrVector;
typedef std::vector< IncludePtr > IncludePtrVector;
typedef std::vector< MacroPtr > MacroPtrVector;
typedef std::vector< MemberInitPtr > MemberInitPtrVector;
typedef std::vector< NamespacePtr > NamespacePtrVector;
typedef std::vector< TemplateParmPtr > TemplateParmPtrVector;
typedef std::vector< TokenPtr > TokenPtrVector;
typedef std::vector< TypedefPtr > TypedefPtrVector;
typedef std::vector< TypeNamePtr > TypeNamePtrVector;
typedef std::vector< TypeSpecPtr > TypeSpecPtrVector;
typedef std::vector< UsingPtr > UsingPtrVector;

typedef std::vector< Class* > ClassVector;
typedef std::vector< CxxNamed* > NamedVector;
typedef std::vector< Data* > DataVector;
typedef std::vector< Elif* > ElifVector;
typedef std::vector< Enum* > EnumVector;
typedef std::vector< Forward* > ForwardVector;
typedef std::vector< Function* > FunctionVector;
typedef std::vector< Macro* > MacroVector;
typedef std::vector< StackArg > StackArgVector;
typedef std::vector< Typedef* > TypedefVector;
typedef std::vector< Using* > UsingVector;

typedef std::set< std::string > stringSet;
typedef std::set< const CxxNamed* > CxxNamedSet;
}
#endif