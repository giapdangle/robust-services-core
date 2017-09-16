//==============================================================================
//
//  Parser.h
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include <string>
#include <cstddef>
#include "CodeTypes.h"
#include "Cxx.h"
#include "CxxFwd.h"
#include "Lexer.h"
#include "SysFile.h"
#include "SysTime.h"
#include "SysTypes.h"

using namespace NodeBase;

//------------------------------------------------------------------------------

namespace CodeTools
{
//  C++ parser, implemented using recursive descent.  Its purpose is to support
//  software analysis, so it assumes that it is parsing code that successfully
//  compiles and links.  It can therefore accept constructs that would actually
//  be illegal.  Conversely, it can reject legal constructs that the code base
//  does not use.  This keeps the grammar manageable and, in some cases, rejects
//  unwanted constructs.
//
//  NOT SUPPORTED
//  -------------
//  C++ specifications are voluminous, so there are many things that the parser
//  does not support, including some things in C++11 and beyond.  The following
//  is a list of pre-C++11 things that are known not to be supported, along with
//  some of the functions that would need to be modified to support them.
//
//  reserved words:
//    o asm, goto, register, volatile, wchar_t
//    o and, and_eq, bitand, bitor, compl, not, not_eq, or, or_eq, xor, xor_eq
//    o #undef, #line, #pragma (parsed but have no effect)
//    o #if, #elif (the code that follows the directive is ignored)
//    o elaborated type specifiers (class, struct, union, or enum prefixed to
//      resolve a type ambiguity caused by overloading an identifier)
//  character and string literals (GetCxxExpr, GetCxxAlpha, GetChar, GetStr):
//    o continuing a string literal when quotation marks are separated by spaces
//    o type tags (u8, u, U, L, R)
//  namespaces:
//    o anonymous namespaces (GetNamespace)
//    o namespace aliases (GetNamespace)
//  classes:
//    o multiple inheritance (GetBaseDecl)
//    o tagging a base class as virtual (GetBaseDecl)
//    o defining a class within a function (ParseInBlock and others)
//    o non-public base class (allowed by parser, but accessibility checking
//      does not enforce it)
//    o anonymous structs (GetClassDecl)
//    o enums, typedefs, or functions in an anonymous union (allowed by parser,
//      but CxxArea.FindEnum, FindFunc, and FindType do not look for them)
//    o including a union instance immediately after declaring it (GetClassDecl)
//  functions:
//    o the order of tags is inflexible: "extern inline static virtual explicit
//      constexpr <function-definition> const noexcept override" (GetFuncDecl)
//    o using a different type (an alias) for an argument in the definition of
//      a previously declared function (DataSpec.MatchesExactly)
//    o argument-dependent lookup of regular functions (done only for operator
//      overloads)
//    o overloading the function call or comma operator (the parser allows it,
//      but calls to the overload won't be registered because Operator.Execute
//      doesn't look for it)
//    o variadic argument lists
//    o dynamic exception specifications
//  data:
//    o the order of tags is inflexible: "extern static mutable constexpr const"
//      (GetClassData, GetSpaceData, GetFuncData)
//    o declaring more than one data instance in the same statement, either at
//      file scope or within a class (GetClassData and GetSpaceData)--note that
//      this *is* supported within a function (e.g. int i = 0, *pi = nullptr)
//    o unnamed bit fields (GetClassData)
//  typedefs:
//    o "typedef enum" and "typedef struct" (GetTypedef)
//  templates:
//    o template parameters other than typename, class, or struct: in the /subs
//      directory, for example, bitset had to be declared as bitset<typename N>
//      instead of bitset<size_t N>
//    o template arguments other than qualified names: bitset<sizeof(uint8_t)>,
//      for example, would have to be written as bitset<bytesize>, following
//      the definition const size_t bytesize = sizeof(uint8_t)
//    o a constructor call that requires template argument deduction when a
//      template is a base class: need to include the template argument after
//      the class template name
//  C++11:
//    o alignas, alignof, char16_t, char32_t, concept, decltype, export, final,
//      requires, static_assert, thread_local
//    o constructor inheritance (GetUsing, Class.FindCtor, and others)
//    o scoped enums (GetEnum)
//    o type aliases and alias templates (GetUsing and others)
//    o lambdas (GetArgument and many others)
//    o range-based for loops (GetFor and many others)
//
//  Comments in the CodeTools namespace that begin with "//c" describe other
//  enhancements that have not been implemented.
//
//  WORKAROUNDS
//  -----------
//  o See the above comments, primarily for templates.
//  o If two types with the same name are used as arguments to the same template
//    class, name resolution can pick the wrong template instance.  For example,
//    T<C1::A> and T<C2::A>, where A is defined in two different scopes.  This
//    occurs because template arguments are not yet considered when determining
//    the visibility of a template instance.  The workaround is to ensure that
//    all template arguments to a given template have different names.
//  o If template T with parameter P is instantiated with argument A, and T<P>
//    appears within one of its functions, it causes a parse error because it
//    gets expanded to T<A><A>.  This occurs because T is first replaced by
//    T<A>, after which <P> is replaced by <A>.  The workaround is to remove
//    the <P>, which is not required.
//
class Parser
{
public:
   //  Creates a parser that will use the options specified in OPTS.
   //
   explicit Parser(const std::string& opts);

   //  Creates a parser that will parse a code fragment on behalf of another
   //  parser.  The scope in which to parse must be provided.
   //
   explicit Parser(CxxScope* scope);

   //  Not subclassed.
   //
   ~Parser();

   //  Parses FILE.  Returns true on success.
   //
   bool Parse(CodeFile& file);

   //  Parses a class template instance starting at POS.
   //
   bool ParseClassInst(ClassInst* inst, size_t pos);

   //  Parses the function template instance identified by NAME, adding it to
   //  the namespace or class specified by AREA.  TYPE and CODE contain the
   //  instance's template arguments and code.
   //
   bool ParseFuncInst(const std::string& name,
      const TypeName* type, CxxArea* area, const stringPtr& code);

   //  Returns true and creates SPEC if CODE is a valid type specification.
   //
   bool ParseTypeSpec(const std::string& code, TypeSpecPtr& spec);

   //  If parsing a template instance for which NAME is an argument, returns
   //  that argument.
   //
   CxxNamed* ResolveInstanceArgument(const QualName* name) const;

   //  Returns the time when the parse originally started.
   //
   static const SysTime* GetTime();

   //  Returns a string that specifies the parser's current position.  If
   //  parsing source code, this will be a numeric.  If parsing a template,
   //  it provides the template's name.
   //
   std::string GetPos() const;

   //  Returns the parser's current position within its Lexer.
   //
   size_t GetCurr() const { return lexer_.Curr(); }

   //  Returns the parser's previous position within its Lexer.
   //
   size_t GetPrev() const { return lexer_.Prev(); }

   //  Returns true if a template instance is currently being parsed.
   //
   bool ParsingTemplateInstance()
      const { return (tmpltClassInst_ | tmpltFuncInst_); }
private:
   //  Parses declarations at file scope.  SPACE is the current namespace.
   //
   void GetFileDecls(Namespace* space);

   //  Tries the parses associated with KWD, adding declarations to SPACE.
   //
   bool ParseInFile(Cxx::Keyword kwd, Namespace* space);

   //  Parsing of preprocessor directives.  HandleDirective is used when a '#'
   //  is encountered.  When handling a #define or #include, it returns true on
   //  success, but DIR is nonetheless nullptr.  The other functions handle the
   //  directive after which they are named.  Those with a DIR argument create
   //  and return a directive so that it can be added to the current scope.  A
   //  #define or #include is always added to the current file, regardless of
   //  the scope in which it appears.
   //
   bool HandleDirective(DirectivePtr& dir);
   bool HandleInclude();
   bool HandleDefine();
   bool HandleElif(DirectivePtr& dir);
   bool HandleElse(DirectivePtr& dir);
   bool HandleEndif(DirectivePtr& dir);
   bool HandleError(DirectivePtr& dir);
   bool HandleIf(DirectivePtr& dir);
   bool HandleIfdef(DirectivePtr& dir);
   bool HandleIfndef(DirectivePtr& dir);
   bool HandleLine(DirectivePtr& dir);
   bool HandlePragma(DirectivePtr& dir);
   bool HandleUndef(DirectivePtr& dir);

   //  Returns true and creates EXPR on finding a preprocessor expression.
   //  END delimits the expression, which should finish at END - 1.  The
   //  expression is evaluated when END is reached.
   //
   bool GetPreExpr(ExprPtr& expr, size_t end);

   //  Updates EXPR with the results of parsing an expression upon reaching
   //  an alphabetic character.  Used in preprocessor directives.
   //
   bool GetPreAlpha(ExprPtr& expr);

   //  Parses declarations in CLS (a class, struct, or union).
   //
   void GetMemberDecls(Class* cls);

   //  Tries the parses associated with KWD, adding declarations to CLS.
   //
   bool ParseInClass(Cxx::Keyword kwd, Class* cls);

   //  Parses statements in BLOCK (a code block).  BRACED is set if the
   //  block started with a left brace.  If it didn't, it contains only
   //  one statement.
   //
   bool GetStatements(BlockPtr& block, bool braced);

   //  Tries the parses associated with KWD, adding statements to BLOCK.
   //
   bool ParseInBlock(Cxx::Keyword kwd, Block* block);

   //  Returns true and creates USE on finding a using declaration.
   //
   bool GetUsing(UsingPtr& use);

   //  Returns true on finding a namespace declaration.
   //
   bool GetNamespace();

   //  Returns true on finding a class declaration.  KWD is the keyword
   //  that was just found, and AREA is the namespace or class in which
   //  the class is being declared.
   //
   bool GetClass(Cxx::Keyword kwd, CxxArea* area);

   //  Returns true and creates CLS on finding a class declaration.  If it
   //  is only a forward declaration, it is returned in FORW.  KWD is the
   //  keyword that was just found.
   //
   bool GetClassDecl(Cxx::Keyword kwd, ClassPtr& cls, ForwardPtr& forw);

   //  Returns true and creates BASE on finding a base class declaration.
   //
   bool GetBaseDecl(BaseDeclPtr& base);

   //  Returns true after successfully parsing CLS's inlines.
   //
   bool GetInlines(Class* cls);

   //  Returns true and creates DECL on finding a friend declaration.
   //
   bool GetFriend(FriendPtr& decl);

   //  Returns true and updates ACCESS on finding an access scope declaration.
   //  KWD is the keyword that was just found.
   //
   bool GetAccess(Cxx::Keyword kwd, Cxx::Access& access);

   //  Returns true and creates DECL on finding an enum.
   //
   bool GetEnum(EnumPtr& decl);

   //  Returns true on finding an enumerator defined by NAME and INIT.
   //
   bool GetEnumerator(std::string& name, ExprPtr& init);

   //  Returns true and creates TDEF on finding a typedef.
   //
   bool GetTypedef(TypedefPtr& type);

   //  Returns true and updates FUNC on finding a function specification.
   //  SPEC is the function's return type, which has already been parsed.
   //
   bool GetFuncSpec(TypeSpecPtr& spec, FunctionPtr& func);

   //  Returns true and creates DATA on finding a data declaration
   //  o at file scope
   //  o within a class
   //  o within a function
   //  KWD is the keyword that was found at the start of the possible data
   //  declaration, but the lexer has not consumed it.
   //
   bool GetSpaceData(Cxx::Keyword kwd, DataPtr& data);
   bool GetClassData(DataPtr& data);
   bool GetFuncData(DataPtr& data);

   //  Returns true and creates FUNC on finding a function declaration.
   //  KWD is the keyword that was found at the start of the potential
   //  function declaration, but the parse has not advanced over it.
   //
   bool GetFuncDecl(Cxx::Keyword kwd, FunctionPtr& func);

   //  Returns true and creates FUNC on finding a function definition.
   //  KWD is the keyword that was found at the start of the potential
   //  function definition, but the parse has not advanced over it.
   //
   bool GetFuncDefn(Cxx::Keyword kwd, FunctionPtr& func);

   //  Returns true and updates FUNC on finding a function implementation.
   //
   bool GetFuncImpl(Function* func);

   //  Returns true and creates FUNC on finding
   //  o GetCtorDecl: a constructor declaration
   //  o GetCtorDefn: a constructor implementation
   //  o GetCtorInit: a constructor initialization list
   //  o GetDtorDecl: a destructor declaration
   //  o GetDtorDefn: a destructor implementation
   //  o GetProcDecl: a function declaration
   //  o GetProcDefn: a function implementation
   //
   bool GetCtorDecl(FunctionPtr& func);
   bool GetCtorDefn(FunctionPtr& func);
   bool GetCtorInit(FunctionPtr& func);
   bool GetDtorDecl(FunctionPtr& func);
   bool GetDtorDefn(FunctionPtr& func);
   bool GetProcDecl(FunctionPtr& func);
   bool GetProcDefn(FunctionPtr& func);

   //  Returns true and creates SPEC on finding a type specification.  If SPEC
   //  is a function type (FuncSpec), the second version returns the name, if
   //  any, assigned to that type.
   //
   bool GetTypeSpec(TypeSpecPtr& spec);
   bool GetTypeSpec(TypeSpecPtr& spec, std::string& name);

   //  Returns true and creates ARRAY on finding an array specification.
   //
   bool GetArraySpec(ArraySpecPtr& array);

   //  Returns true and updates FUNC on finding one or more function arguments.
   //
   bool GetArguments(FunctionPtr& func);

   //  Returns true and creates DECL on finding a function argument.
   //
   bool GetArgument(ArgumentPtr& arg);

   //  Returns true and creates PARMS on finding a template declaration.
   //
   bool GetTemplateParms(TemplateParmsPtr& parms);

   //  Returns true and creates PARM on finding a template parameter.
   //
   bool GetTemplateParm(TemplateParmPtr& parm);

   //  Returns true and creates or updates NAME on finding a name that could be
   //  qualified.  If NAME ends in "operator", the operator that follows it is
   //  also parsed.
   //
   bool GetQualName(QualNamePtr& name);

   //  Checks if NAME is a built-in type or a keyword that is an invalid type.
   //
   bool CheckType(QualNamePtr& name);

   //  Invoked when NAME (of TYPE) began with long, short, signed, or unsigned.
   //
   bool GetCompoundType(QualNamePtr& name, Cxx::Type type);

   //  Invoked by GetCompoundType when NAME ended with TYPE.  SIZE and SIGN
   //  are non-zero if NAME was tagged as long, short, signed, or unsigned.
   //
   static bool SetCompoundType
      (QualNamePtr& name, Cxx::Type type, int size, int sign);

   //  Returns true and creates or updates TYPE on finding a typed name, which
   //  may include a template signature.
   //
   bool GetTypeName(TypeNamePtr& type);

   //  Returns true and creates NAME on finding an identifier.  This is used to
   //  to parse constructor and destructor names, in which the contents of angle
   //  brackets are included in the name when parsing a template instance.
   //
   bool GetName(std::string& name);

   //  Updates SPEC with pointers ('*') that follow the current parse
   //  location.
   //
   void GetPointers(TypeSpec* spec);

   //  Updates SPEC with references ('&') that follow the current parse
   //  location.
   //
   void GetReferences(TypeSpec* spec);

   //  Returns the number of pointers ('*') that follow the current parse
   //  location.
   //
   size_t GetPointers();

   //  Returns true and creates BLOCK on finding a code block.
   //
   bool GetBlock(BlockPtr& block);

   //  Returns true and creates EXPR on finding an expression.  END delimits
   //  the expression, which should finish at END - 1.  If FORCE is set, the
   //  expression is evaluated when END is reached.
   //
   bool GetCxxExpr(ExprPtr& expr, size_t end, bool force = true);

   //  Returns true and creates EXPR on finding an expression enclosed by
   //  parentheses.  If OMIT is set, the left parenthesis has already been
   //  parsed; otherwise, it must be the next character.  If OPT is set,
   //  the expression is optional.
   //
   bool GetParExpr(ExprPtr& expr, bool omit, bool opt = false);

   //  Updates EXPR with the results of parsing an expression upon reaching
   //  an alphabetic character.
   //
   bool GetCxxAlpha(ExprPtr& expr);

   //  Updates EXPR with the results of parsing an expression upon reaching
   //  a punctuation character.  CXX is true when parsing C++ and is false
   //  when parsing preprocessor directives.
   //
   bool GetOp(ExprPtr& expr, bool cxx);

   //  Updates EXPR with the results of parsing a literal.
   //
   bool GetNum(ExprPtr& expr);
   bool GetChar(ExprPtr& expr);
   bool GetStr(ExprPtr& expr);

   //  Updates EXPR with the results of parsing an expression enclosed by
   //  parentheses.  It tries, in order,
   //  o GetArgList to handle a function call,
   //  o GetCast to handle a C-style cast, and
   //  o GetPrecedence to handle parentheses that the order of evaluation.
   //
   bool HandleParentheses(ExprPtr& expr);
   bool GetArgList(TokenPtr& call);
   bool GetCast(ExprPtr& expr);
   bool GetPrecedence(ExprPtr& expr);

   //  Updates EXPR with the results of parsing an expression that begins
   //  with a '~', which could either be a ones complement operator or a
   //  direct destructor invocation.  START was the location of the '~',
   //  which has already been parsed as an operator.
   //
   bool HandleTilde(ExprPtr& expr, size_t start);

   //  Updates EXPR with the results of parsing an array index.
   //
   bool GetSubscript(ExprPtr& expr);

   //  Updates EXPR with the results of parsing a brace initialization list.
   //
   bool GetBraceInit(ExprPtr& expr);

   //  Updates EXPR with the results of parsing an expression that begins with
   //  the specified operator.
   //
   bool GetCxxCast(ExprPtr& expr, Cxx::Operator op);
   bool GetConditional(ExprPtr& expr);
   bool GetDefined(ExprPtr& expr);
   bool GetDelete(ExprPtr& expr, Cxx::Operator op);
   bool GetNew(ExprPtr& expr, Cxx::Operator op);
   bool GetSizeOf(ExprPtr& expr);
   bool GetThrow(ExprPtr& expr);
   bool GetTypeId(ExprPtr& expr);

   //  Updates EXPR with the results of parsing a statement that begins with
   //  the specified keyword.  GetBasic handles assignments, function calls,
   //  and null statements.
   //
   bool GetBasic(TokenPtr& expr);
   bool GetBreak(TokenPtr& expr);
   bool GetCase(TokenPtr& expr);
   bool GetCatch(TokenPtr& expr);
   bool GetContinue(TokenPtr& expr);
   bool GetDefault(TokenPtr& expr);
   bool GetDo(TokenPtr& expr);
   bool GetFor(TokenPtr& expr);
   bool GetIf(TokenPtr& expr);
   bool GetReturn(TokenPtr& expr);
   bool GetSwitch(TokenPtr& expr);
   bool GetTry(TokenPtr& expr);
   bool GetWhile(TokenPtr& expr);

   //  Prepares to parse CODE.  PREPROCESS is set if the code should be
   //  preprocessed.
   //
   void Enter(const std::string& code, bool preprocess);

   //  Updates STR to the next keyword.  Returns Cxx::NIL_KEYWORD if the
   //  next token is not a keyword.
   //
   Cxx::Keyword NextKeyword(std::string& str);

   //  Returns true if the next keyword is STR.
   //
   bool NextKeywordIs(fixed_string str);

   //  Invokes SetScope, SetAccess, and SetDecl on ITEM immediately after
   //  its creation.
   //
   void SetContext(CxxNamed* item, size_t pos) const;

   //  Reasons for errors associated with preprocessor directives.
   //
   enum ErrorCode
   {
      DirectiveMismatch = 1,
      SymbolExpected,
      FileExpected,
      ConditionExpected,
      ElifUnexpected,
      ElseUnexpected,
      EndifUnexpected,
      EndifExpected
   };

   //  Reports an error associated with a preprocessor directive and returns
   //  false.
   //
   static bool Report(ErrorCode code);

   //  Logs WARNING at POS.  If POS is not specified, the last position where
   //  parsing started is used.
   //
   void Log(Warning warning, size_t pos = std::string::npos) const;

   //  When the parsing of an expression fails, this is invoked to add the
   //  unparsed string to EXPR.  The string extends from the current parse
   //  string to END.  A "<@" prefix and "@>" suffix are also added to this
   //  string.  Returns false.
   //
   bool Punt(ExprPtr& expr, size_t end);

   //  Backs up to POS, deletes FUNC, and returns false.  FUNC needs to be
   //  deleted immediately when it may have pushed a new scope, which must
   //  be popped in case another function parse is attempted.  If that new
   //  parse succeeds, the Function constructor invokes OpenScope, picking
   //  up the scope set by the function that failed to parse.  This occurs
   //  because the failed function would normally not be deleted until its
   //  FunctionPtr releases it to acquire the new function, which can only
   //  occur *after* the new function has been constructed.
   //
   bool Retreat(size_t pos, FunctionPtr& func);

   //  Returns true from the function named FN, which began its parse at
   //  START, after adding its parse string (START to prev_) to log_.
   //
   bool Success(const std::string& fn, size_t start) const;

   //  Returns a string of blanks based on the depth of parsing.
   //
   static std::string Indent();

   //  Overridden to prohibit copying.
   //
   Parser(const Parser& that);
   void operator=(const Parser& that);

   //  The lexical analyzer.
   //
   Lexer lexer_;

   //  Compiler options.
   //
   std::string opts_;

   //  The time when the parse started.
   //
   const SysTime time_;

   //  The stack depth at which Parse() was invoked.
   //
   size_t depth_;

   //  The position where the most recent keyword began.
   //
   size_t kwdBegin_;

   //  Set when parsing a class template instance.
   //
   bool tmpltClassInst_;

   //  Set when parsing a function template instance.
   //
   bool tmpltFuncInst_;

   //  The template arguments when parsing a template instance.
   //
   const TypeName* type_;

   //  The name of the template being parsed.
   //
   std::string tmpltName_;

   //  Output file for parse tracing, if any.
   //
   ostreamPtr pTrace_;
};
}
#endif