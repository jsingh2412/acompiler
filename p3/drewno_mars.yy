%skeleton "lalr1.cc"
%require "3.0"
%debug
%defines
%define api.namespace{drewno_mars}
%define api.parser.class {Parser}
%define parse.error verbose
%output "parser.cc"
%token-table

%code requires{
	#include <list>
	#include "tokens.hpp"
	#include "ast.hpp"
	namespace drewno_mars {
		class Scanner;
	}

//The following definition is required when 
// we don't use the %locations directive (which we won't)
# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

//End "requires" code
}

%parse-param { drewno_mars::Scanner &scanner }
%parse-param { drewno_mars::ProgramNode** root }
%code{
   // C std code for utility functions
   #include <iostream>
   #include <cstdlib>
   #include <fstream>

   // Our code for interoperation between scanner/parser
   #include "scanner.hpp"
   #include "ast.hpp"
   #include "tokens.hpp"

  //Request tokens from our scanner member, not 
  // from a global function
  #undef yylex
  #define yylex scanner.yylex
}

/*
The %union directive is a way to specify the 
set of possible types that might be used as
translation attributes on a symbol.
For this project, only terminals have types (we'll
have translation attributes for non-terminals in the next
project)
*/
%union {
   drewno_mars::Token*                         transToken;
   drewno_mars::Token*                         lexeme;
   drewno_mars::IDToken*                       transIDToken;
   drewno_mars::StrToken*					   transStrToken;
   drewno_mars::IntLitToken*				   transIntToken;
   drewno_mars::ProgramNode*                   transProgram;
   drewno_mars::DeclNode *                     transDecl;
   std::list<drewno_mars::DeclNode *> *        transDeclList;
   drewno_mars::VarDeclNode *                  transVarDecl;
   drewno_mars::TypeNode *                     transType;
   drewno_mars::LocNode *                      transLoc;
   drewno_mars::IDNode *                       transID;
   drewno_mars::ExpNode * 					   transExp;
   drewno_mars::IntLitNode *				   transIntLit;
   drewno_mars::ClassDefnNode * 			   transClassDecl;
   drewno_mars::StmtNode * 					   transStmt;
   drewno_mars::FnDeclNode * 				   transFnDecl;
   drewno_mars::CallExpNode *				   transCallExp;
   drewno_mars::ClassDefnNode * 			   transClassDef;
   std::list<drewno_mars::ExpNode *> *         transExpList;
   std::list<drewno_mars::StmtNode *> *		   transStmtList;
   drewno_mars::FormalDeclNode * 		  	   transFormalDecl;
   std::list<drewno_mars::FormalDeclNode *> *  transFormalsList;
}

%define parse.assert

/* Terminals 
 *  No need to touch these, but do note the translation type
 *  of each node. Most are just "transToken", which is defined in
 *  the %union above to mean that the token translation is an instance
 *  of drewno_mars::Token *, and thus has no fields (other than line and column).
 *  Some terminals, like ID, are "transIDToken", meaning the translation
 *  also has a name field. 
*/
%token                   END	   0 "end file"
%token	<transToken>     AND
%token	<transToken>     ASSIGN
%token	<transToken>     BOOL
%token	<transToken>     COLON
%token	<transToken>     COMMA
%token	<transToken>     CLASS
%token	<transToken>     DASH
%token	<transToken>     ELSE
%token	<transToken>     EXIT
%token	<transToken>     EQUALS
%token	<transToken>     FALSE
%token	<transToken>     GIVE
%token	<transToken>     GREATER
%token	<transToken>     GREATEREQ
%token	<transIDToken>   ID
%token	<transToken>     IF
%token	<transToken>     INT
%token	<transIntToken>  INTLITERAL
%token	<transToken>     LCURLY
%token	<transToken>     LESS
%token	<transToken>     LESSEQ
%token	<transToken>     LPAREN
%token	<transToken>     MAGIC
%token	<transToken>     NOT
%token	<transToken>     NOTEQUALS
%token	<transToken>     OR
%token	<transToken>     PERFECT
%token	<transToken>     CROSS
%token	<transToken>     POSTDEC
%token	<transToken>     POSTINC
%token	<transToken>     RETURN
%token	<transToken>     RCURLY
%token	<transToken>     RPAREN
%token	<transToken>     SEMICOL
%token	<transToken>     SLASH
%token	<transToken>     STAR
%token	<transStrToken>  STRINGLITERAL
%token	<transToken>     TAKE
%token	<transToken>     TRUE
%token	<transToken>     VOID
%token	<transToken>     WHILE


/* Nonterminals
*  The specifier in angle brackets
*  indicates the type of the translation attribute using
*  the names defined in the %union directive above
*  TODO: You will need to add more attributes for other nonterminals
*  to this list as you add productions to the grammar
*  below (along with indicating the appropriate translation
*  attribute type).
*/
/*    (attribute type)    (nonterminal)    */
%type <transProgram> program
%type <transDeclList> globals
%type <transDecl> decl
%type <transVarDecl> varDecl
%type <transType> type
%type <transType> primType
%type <transLoc> loc
%type <transID> id
%type <transExp> exp
%type <transClassDecl> classDecl
%type <transStmt> stmt
%type <transFnDecl> fnDecl
%type <transCallExp> callExp
%type <transExp> term
%type <transClassDef> classBody
%type <transExpList> actualsList
%type <transStmtList> stmtList
%type <transFormalDecl> formalDecl
%type <transFormalsList> formals
%type <transFormalsList> formalsList
%type <transStmt> blockStmt

%right ASSIGN
%left OR
%left AND
%nonassoc LESS GREATER LESSEQ GREATEREQ EQUALS NOTEQUALS
%left DASH CROSS
%left STAR SLASH
%left NOT 

%%

program 	: globals
		  {
		  $$ = new ProgramNode($1);
		  *root = $$;
		  }

globals 	: globals decl
	  	  { 
		  $$ = $1;
		  DeclNode * declNode = $2;
		  $$->push_back(declNode);
	  	  }
		| /* epsilon */
		  {
		  $$ = new std::list<DeclNode *>();
		  }

decl 		: varDecl SEMICOL
		  {
		  $$ = $1;
		  }
 		| classDecl { $$ = $1; }
 		| fnDecl { $$ = $1; }

varDecl 	: id COLON type
		  {
		  const Position * p;
		  p = new Position($1->pos(), $2->pos());
		  $$ = new VarDeclNode(p, $1, $3);
		  }
		| id COLON type ASSIGN exp
		  {
		  const Position * p;
		  p = new Position($1->pos(), $2->pos());
		  $$ = new VarDeclNode(p, $1, $3, $5);
		  }

type		: primType
		  {
		  $$ = $1;
		  }
		| id
		  {
			
		  }
		| PERFECT primType
		  {
			
		  }
		| PERFECT id
		  {
			
		  }

primType 	: INT
	  	  { 
		  $$ = new IntTypeNode($1->pos());
		  }
		| BOOL
		  {
		  $$ = new BoolTypeNode($1->pos());
		  }
		| VOID
		  {
		  $$ = new VoidTypeNode($1->pos());	
		  }

classDecl	: id COLON CLASS LCURLY classBody RCURLY SEMICOL
		  {
			
		  }

classBody	: classBody varDecl SEMICOL
		  {
			$$ = $1;
		  	DeclNode * declNode = $2;
		  	$$->push_back(declNode);
		  }
		| classBody fnDecl
		  {
			$$ = $1;
		  	DeclNode * declNode = $2;
		  	$$->push_back(declNode);
		  }
		| /* epsilon */
		  {
			
		  }

fnDecl  : id COLON LPAREN formals RPAREN type LCURLY stmtList RCURLY
		  {
			const Position * pos;
			pos = new Position($1->pos(), $6->pos());
			$$ = new FnDeclNode($1->pos(), $6, $1, $4, $8);
		  }

formals 	: /* epsilon */
		  {
			$$ = new std::list<FormalDeclNode *>();
		  }
		| formalsList
		  {
			$$ = $1;
		  }

formalsList 	: formalDecl
		  {
			FormalDeclNode * fDecl = $1;
			$$->push_back(fDecl);
		  }
		| formalsList COMMA formalDecl
		  {
			$$ = $1;
			FormalDeclNode * fDecl = $3;
			$$->push_back(fDecl);
		  }

formalDecl 	: id COLON type
		  {
			Position * p = new Position($1->pos(), $2->pos());
			$$ = new FormalDeclNode(p, $1, $3);
		  }

stmtList 	: /* epsilon */
	   	  {
			$$ = new  std::list<StmtNode *>(); 
	   	  }
		| stmtList stmt SEMICOL
	  	  {
			$$ = $1;
			StmtNode * stmt = $2;
			$$->push_back(stmt);
	  	  }
		| stmtList blockStmt
	  	  {
			$$ = $1;
			StmtNode * stmt = $2;
			$$->push_back(stmt);
	  	  }

blockStmt	: WHILE LPAREN exp RPAREN LCURLY stmtList RCURLY
		  {
			const Position * pos;
			pos = new Position($1->pos(), $2->pos());
			$$ = new WhileStmtNode(pos, $3, $6);
		  }
		| IF LPAREN exp RPAREN LCURLY stmtList RCURLY
		  {

		  }
		| IF LPAREN exp RPAREN LCURLY stmtList RCURLY ELSE LCURLY stmtList RCURLY
		  {

		  }

stmt		: varDecl
		  {
			$$ = $1;
		  }
		| loc ASSIGN exp
		  {
			const Position * pos = $1->pos();
			$$ = new AssignStmtNode(pos, $1, $3);
		  }
		| loc POSTDEC
		  {
			const Position * pos = $1->pos();
			$$ = new PostDecStmtNode(pos, $1);
		  }
		| loc POSTINC
		  {
			const Position * pos = $1->pos();
			$$ = new PostIncStmtNode(pos, $1);
		  }
		| GIVE exp
		  {
			const Position * pos = $1->pos();
			$$ = new GiveStmtNode(pos, $2);
		  }
		| TAKE loc
		  {
			const Position * pos = $1->pos();
			$$ = new TakeStmtNode(pos, $2);
		  }
		| RETURN exp
		  {
			const Position * pos = $1->pos();
			$$ = new ReturnStmtNode(pos, $2);
		  }
		| RETURN
		  {
			const Position * pos = $1->pos();
			$$ = new ReturnStmtNode(pos);
		  }
		| EXIT
		  {
			const Position * pos = $1->pos();
			$$ = new ExitStmtNode(pos);
		  }
		| callExp
		  { 
			const Position * pos = $1->pos();
			$$ = new CallStmtNode(pos, $1);
		  }

exp		: exp DASH exp
	  	  {
			const Position * pos = $1->pos();
			$$ = new MinusNode(pos, $1, $3);
		  }
		| exp CROSS exp
	  	  {
			const Position * pos = $1->pos();
			$$ = new PlusNode(pos, $1, $3);
		  }
		| exp STAR exp
	  	  {
			const Position * pos = $1->pos();
			$$ = new TimesNode(pos, $1, $3);
		  }
		| exp SLASH exp
	  	  {
			const Position * pos = $1->pos();
			$$ = new DivideNode(pos, $1, $3);
		  }
		| exp AND exp
	  	  {
			const Position * pos = $1->pos();
			$$ = new AndNode(pos, $1, $3);
		  }
		| exp OR exp
	  	  {
			const Position * pos = $1->pos();
			$$ = new OrNode(pos, $1, $3);
		  }
		| exp EQUALS exp
	  	  {
			const Position * pos = $1->pos();
			$$ = new EqualsNode(pos, $1, $3);
		  }
		| exp NOTEQUALS exp
	  	  {
			const Position * pos = $1->pos();
			$$ = new NotEqualsNode(pos, $1, $3);
		  }
		| exp GREATER exp
	  	  {
			const Position * pos = $1->pos();
			$$ = new GreaterNode(pos, $1, $3);
		  }
		| exp GREATEREQ exp
	  	  {
			const Position * pos = $1->pos();
			$$ = new GreaterEqNode(pos, $1, $3);
		  }
		| exp LESS exp
	  	  {
			const Position * pos = $1->pos();
			$$ = new LessNode(pos, $1, $3);
		  }
		| exp LESSEQ exp
	  	  {
			const Position * pos = $1->pos();
			$$ = new LessEqNode(pos, $1, $3);
		  }
		| NOT exp
	  	  {
			const Position * pos = $1->pos();
			$$ = new NegNode(pos, $2);
		  }
		| DASH term
	  	  {
			const Position * pos = $1->pos();
			$$ = new NotNode(pos, $2);
		  }
		| term
	  	  {
			$$ = $1;
		  }

callExp		: loc LPAREN RPAREN
		  {
			const Position * pos = $1->pos();
			$$ = new CallExpNode(pos, $1);
		  }
		| loc LPAREN actualsList RPAREN
		  {
			const Position * pos = $1->pos();
			$$ = new CallExpNode(pos, $1);
		  }

actualsList	: exp
		  {
			$$ = new std::list<drewno_mars::ExpNode *>();
			ExpNode * inExp = $1;
			$$->push_back(inExp);
		  }
		| actualsList COMMA exp
		  {
			$$ = $1;
			ExpNode * inExp = $3;
			$$->push_back(inExp);
		  }

term 		: loc
		  { 
			$$ = $1;
		  }
		| INTLITERAL 
		  {
			const Position * pos = $1->pos();
			$$ = new IntLitNode(pos, $1->num());
		  }
		| STRINGLITERAL 
		  {
			const Position * pos = $1->pos();
			$$ = new StrLitNode(pos, $1->str());
		  }
		| TRUE
		  {
			const Position * pos = $1->pos();
			$$ = new TrueNode(pos);
		  }
		| FALSE
		  {
			const Position * pos = $1->pos();
			$$ = new FalseNode(pos);
		  }
		| MAGIC
		  {
			const Position * pos = $1->pos();
			$$ = new MagicNode(pos);
		  }
		| LPAREN exp RPAREN
		  {
			const Position * pos = $2->pos();
			$$ = $2;
		  }
		| callExp
		  {
			$$ = $1;
		  }

loc		: id
		  {
		  $$ = $1;
		  }
		| loc POSTDEC id
		  {
			$$ = $1;
			
		  }

id		: ID
		  {
		  const Position * pos = $1->pos();
		  $$ = new IDNode(pos, $1->value());
		  }
	
%%

void drewno_mars::Parser::error(const std::string& msg){
	std::cout << msg << std::endl;
	std::cerr << "syntax error" << std::endl;
}
