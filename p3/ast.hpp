#ifndef DREWNO_MARS_AST_HPP
#define DREWNO_MARS_AST_HPP

#include <ostream>
#include <list>
#include "tokens.hpp"
#include <cassert>


/* You'll probably want to add a bunch of ASTNode subclasses */

namespace drewno_mars{

/* You may find it useful to forward declare AST subclasses
   here so that you can use a class before it's full definition
*/
class ASTNode;
class ExpNode;
class StmtNode;
class ProgramNode;
class DeclNode;
class TypeNode;
class BinaryExpNode;
class IDNode;
class VarDeclNode;
class LocNode;
class FormalDeclNode;
class CallExpNode;
/** 
* \class ASTNode
* Base class for all other AST Node types
**/
class ASTNode{
public:
	ASTNode(const Position * p) : myPos(p){ }
	virtual void unparse(std::ostream& out, int indent) = 0;
	const Position * pos() { return myPos; }
	std::string posStr() { return pos()->span(); }
protected:
	const Position * myPos = nullptr;
};

/** 
* \class ProgramNode
* Class that contains the entire abstract syntax tree for a program.
* Note the list of declarations encompasses all global declarations
* which includes (obviously) all global variables and struct declarations
* and (perhaps less obviously), all function declarations
**/
class ProgramNode : public ASTNode{
public:
	ProgramNode(std::list<DeclNode *> * globalsIn) ;
	void unparse(std::ostream& out, int indent) override;
private:
	std::list<DeclNode * > * myGlobals;
};

/**  \class TypeNode
* Superclass of nodes that indicate a data type. For example, in 
* the declaration "int a", the int part is the type node (a is an IDNode
* and the whole thing is a DeclNode).
**/
class TypeNode : public ASTNode{
protected:
	TypeNode(const Position * p) : ASTNode(p){
	}
public:
	virtual void unparse(std::ostream& out, int indent) = 0;
};

class StmtNode : public ASTNode{
public:
	StmtNode(const Position * p) : ASTNode(p){ }
	void unparse(std::ostream& out, int indent) override = 0;
};

//StmtNode derivations

class CallStmtNode : public StmtNode{
public:
	CallStmtNode(const Position * p, CallExpNode * inFun): StmtNode(p), 
	myFun(inFun){};
	void unparse(std::ostream& out, int indent) override;
private:
	CallExpNode * myFun; 
};

class ExitStmtNode : public StmtNode{
public:
	ExitStmtNode(const Position * p): StmtNode(p){};
	void unparse(std::ostream& out, int indent) override;
};

class GiveStmtNode : public StmtNode{
public:
	GiveStmtNode(const Position * p, ExpNode * inExp): StmtNode(p), myExp(inExp){};
	void unparse(std::ostream& out, int indent) override;
private:
	ExpNode * myExp;
};

class IfElseStmtNode : public StmtNode{
public:

private:
	ExpNode * myExp;
	std::list<StmtNode * > * ifTrue;
	std::list<StmtNode * > * ifFalse;
};

class IfStmtNode : public StmtNode{
public:
};

class PostDecStmtNode : public StmtNode{
public:
	PostDecStmtNode(const Position * p, LocNode * inLoc): StmtNode(p),
	myLoc(inLoc){};
	void unparse(std::ostream& out, int indent) override;
private:
	LocNode * myLoc;
};

class PostIncStmtNode : public StmtNode{
public:
	PostIncStmtNode(const Position * p, LocNode * inLoc): StmtNode(p),
	myLoc(inLoc){};
	void unparse(std::ostream& out, int indent) override;
private:
	LocNode * myLoc;
};

class ReturnStmtNode : public StmtNode{
public:
	ReturnStmtNode(const Position * p, ExpNode * inExp): StmtNode(p),
	myExp(inExp){};
	ReturnStmtNode(const Position * p): StmtNode(p),
	myExp(NULL){};
	void unparse(std::ostream& out, int indent) override;
private:
	ExpNode * myExp;
};

class TakeStmtNode : public StmtNode{
public:
	TakeStmtNode(const Position * p, LocNode * inLoc): StmtNode(p),
	myLoc(inLoc){};
	void unparse(std::ostream& out, int indent) override;
private:
	LocNode * myLoc;
};

class WhileStmtNode : public StmtNode{
public:
	WhileStmtNode(const Position * p, ExpNode * inExp, std::list<StmtNode * > * inStmts):
	StmtNode(p), myExp(inExp), whileLoop(inStmts){};
	void unparse(std::ostream& out, int indent) override;
private:
	ExpNode * myExp;
	std::list<StmtNode * > * whileLoop;
};

class WriteStmtNode : public StmtNode{
public:
private:
	ExpNode * myExp;
};
/**  \class ExpNode
* Superclass for expression nodes (i.e. nodes that can be used as
* part of an expression).  Nodes that are part of an expression
* should inherit from this abstract superclass.
**/
class ExpNode : public ASTNode{
protected:
	ExpNode(const Position * p) : ASTNode(p){ }
};
/** A memory location. LocNodes subclass ExpNode
 * because they can be used as part of an expression. 
**/
class LocNode : public ExpNode{
public:
	LocNode(const Position * p)
	: ExpNode(p) {}
	void unparse(std::ostream& out, int indent) = 0;
};
/** An identifier. Note that IDNodes subclass
 * LocNode because they are a type of memory location. 
**/
class IDNode : public LocNode{
public:
	IDNode(const Position * p, std::string nameIn) 
	: LocNode(p), name(nameIn){ }
	void unparse(std::ostream& out, int indent);
private:
	/** The name of the identifier **/
	std::string name;
};

/** \class DeclNode
* Superclass for declarations (i.e. nodes that can be used to 
* declare a struct, function, variable, etc).  This base class will 
**/
class DeclNode : public StmtNode{
public:
	DeclNode(const Position * p) : StmtNode(p) { }
	void unparse(std::ostream& out, int indent) override = 0;
};
//derivations of DeclNode
class ClassDefnNode : public DeclNode{
public:
	ClassDefnNode(const Position * p, IDNode * inId, std::list<DeclNode *> * inDecls) :
	DeclNode(p), myId(inId){};
	void unparse(std::ostream& out, int indent) override;
	void push_back(DeclNode * inDecl){
		myDecls->push_back(inDecl);
	}
private:
	IDNode * myId;
	std::list<DeclNode * > * myDecls;
};

class FnDeclNode : public DeclNode{
public:
	void unparse(std::ostream& out, int indent) override;
	FnDeclNode(const Position * p, TypeNode * inType, IDNode * inId, std::list<FormalDeclNode * > * inFormals,
	std::list<StmtNode * > * inStmts): DeclNode(p), myType(inType), funName(inId){}
private:
	TypeNode * myType;
	IDNode * funName;
	std::list<FormalDeclNode * > * myFormals;
	std::list<StmtNode * > * myStmts;
};

/*
class VarDeclNode : public DeclNode{
public:
	VarDeclNode(const Position * p, TypeNode * t, IDNode * i) 
	: DeclNode(p), type(t), id(i){}
	VarDeclNode(const Position * p, TypeNode * t, IDNode * i, ExpNode * e)
	: DeclNode(p), type(t), id(i), exp(e){}
private:
	TypeNode * type;
	IDNode * id;
	ExpNode * exp;
};
*/
/** A variable declaration.
**/
class VarDeclNode : public DeclNode{
public:
	VarDeclNode(const Position * p, IDNode * inID, TypeNode * inType) 
	: DeclNode(p), myID(inID), myType(inType){
		assert (myType != nullptr);
		assert (myID != nullptr);
	}
	VarDeclNode(const Position * p, IDNode * inID, TypeNode * inType, ExpNode * inExp) 
	: DeclNode(p), myID(inID), myType(inType), myExp(inExp){
		assert (myType != nullptr);
		assert (myID != nullptr);
		assert (myExp != nullptr);
	}
	void unparse(std::ostream& out, int indent);
private:
	IDNode * myID;
	TypeNode * myType;
	ExpNode * myExp;
};

class FormalDeclNode : public VarDeclNode{
public:
	FormalDeclNode(const Position * p, IDNode * inID, TypeNode * inType): VarDeclNode(p,inID,inType){}; 
	void unparse(std::ostream& out, int indent);
};

//All ExpNode Derivations
class CallExpNode : public ExpNode{
public:
	CallExpNode(const Position * p, LocNode * inLoc): ExpNode(p), 
	myLoc(inLoc) {};
	void unparse(std::ostream& out, int indent) override;
private:
	LocNode * myLoc;
	std::list<ExpNode * > * myExps;
};

class FalseNode : public ExpNode{
public:
	FalseNode(const Position * p) : ExpNode(p){};
	void unparse(std::ostream& out, int indent) override;
private:
	bool myBool = false;
};

class MagicNode : public ExpNode{
public:
	MagicNode(const Position * p) : ExpNode(p){};
	void unparse(std::ostream& out, int indent) override;
private:
};

class IntLitNode : public ExpNode{
public:
	IntLitNode(const Position * p, int inNum): ExpNode(p), myNum(inNum){};
	void unparse(std::ostream& out, int indent) override;
private:
	int myNum;
};

class StrLitNode : public ExpNode{
public:
	StrLitNode(const Position * p, std::string inStr): ExpNode(p), myStr(inStr){};
	void unparse(std::ostream& out, int indent) override;
private:
	std::string myStr;
};

class TrueNode : public ExpNode{
public:
	TrueNode(const Position * p) : ExpNode(p){};
	void unparse(std::ostream& out, int indent) override;
private:
	bool myBool = true;
};
//derivations of UnaryExpNode
class UnaryExpNode : public ExpNode{
public:
	UnaryExpNode(const Position * p, ExpNode * inExp): ExpNode(p), myExp(inExp){};
	void unparse(std::ostream& out, int indent) override;
private:
	ExpNode * myExp;
};

class NegNode : public UnaryExpNode{
public:
	NegNode(const Position * p, ExpNode * inExp): UnaryExpNode(p, inExp){};
	void unparse(std::ostream& out, int indent) override;
};

class NotNode : public UnaryExpNode{
public:
	NotNode(const Position * p, ExpNode * inExp): UnaryExpNode(p, inExp){};
	void unparse(std::ostream& out, int indent) override;
};

class MemberFieldExpNode : public LocNode{
public:	
	MemberFieldExpNode(const Position * p, LocNode * inLoc, IDNode * inId)
	: LocNode(p), myLoc(inLoc), myId(inId){};
private:
	LocNode * myLoc;
	IDNode * myId;
};
 


//All Type Node Derivations
class IntTypeNode : public TypeNode{
public:
	IntTypeNode(const Position * p) : TypeNode(p){ }
	void unparse(std::ostream& out, int indent);
};

class BoolTypeNode : public TypeNode{
public:
	BoolTypeNode(const Position * p) : TypeNode(p){ }
	void unparse(std::ostream& out, int indent);
};

class ClassTypeNode : public TypeNode{
public:
	ClassTypeNode(const Position * p) : TypeNode(p){ }
	void unparse(std::ostream& out, int indent);
};

class PerfectTypeNode : public TypeNode{
public:
	PerfectTypeNode(const Position * p) : TypeNode(p){ }
	void unparse(std::ostream& out, int indent);
};

class VoidTypeNode : public TypeNode{
public:
	VoidTypeNode(const Position * p) : TypeNode(p){ }
	void unparse(std::ostream& out, int indent);
};

class AssignStmtNode : public StmtNode{
public:
	AssignStmtNode(const Position * p, LocNode * inLoc, ExpNode * inExp): StmtNode(p),
	myLoc(inLoc), myExp(inExp){};
	void unparse(std::ostream& out, int indent);
private:
	LocNode * myLoc;
	ExpNode * myExp;
};

//BinaryExp Node and All Derivations
class BinaryExpNode : public ExpNode{
public:
	BinaryExpNode(const Position * p, ExpNode * lhs_in, ExpNode * rhs_in): ExpNode(p)
	, lhs(lhs_in), rhs(rhs_in){};
	void unparse(std::ostream& out, int indent);
private:
	ExpNode * lhs;
	ExpNode * rhs;
};
//all done under?
class AndNode : public BinaryExpNode{
public:
	AndNode(const Position * p, ExpNode * lhs_in, ExpNode * rhs_in): BinaryExpNode(p, lhs_in, rhs_in){};
	void unparse(std::ostream& out, int indent);
};

class DivideNode : public BinaryExpNode{
public:
	DivideNode(const Position * p, ExpNode * lhs_in, ExpNode * rhs_in): BinaryExpNode(p, lhs_in, rhs_in){};
	void unparse(std::ostream& out, int indent);

};

class EqualsNode : public BinaryExpNode{
public:
	EqualsNode(const Position * p, ExpNode * lhs_in, ExpNode * rhs_in): BinaryExpNode(p, lhs_in, rhs_in){};
	void unparse(std::ostream& out, int indent);
};

class GreaterEqNode : public BinaryExpNode{
public:
	GreaterEqNode(const Position * p, ExpNode * lhs_in, ExpNode * rhs_in): BinaryExpNode(p, lhs_in, rhs_in){};
	void unparse(std::ostream& out, int indent);
};

class GreaterNode : public BinaryExpNode{
public:
	GreaterNode(const Position * p, ExpNode * lhs_in, ExpNode * rhs_in): BinaryExpNode(p, lhs_in, rhs_in){};
	void unparse(std::ostream& out, int indent);
};

class LessEqNode : public BinaryExpNode{
public:
	LessEqNode(const Position * p, ExpNode * lhs_in, ExpNode * rhs_in): BinaryExpNode(p, lhs_in, rhs_in){};
	void unparse(std::ostream& out, int indent);
};

class LessNode : public BinaryExpNode{
public:
	LessNode(const Position * p, ExpNode * lhs_in, ExpNode * rhs_in): BinaryExpNode(p, lhs_in, rhs_in){};
	void unparse(std::ostream& out, int indent);
};

class MinusNode : public BinaryExpNode{
public:
	MinusNode(const Position * p, ExpNode * lhs_in, ExpNode * rhs_in): BinaryExpNode(p, lhs_in, rhs_in){};
	void unparse(std::ostream& out, int indent);
};

class NotEqualsNode : public BinaryExpNode{
public:
	NotEqualsNode(const Position * p, ExpNode * lhs_in, ExpNode * rhs_in): BinaryExpNode(p, lhs_in, rhs_in){};
	void unparse(std::ostream& out, int indent);
};

class OrNode : public BinaryExpNode{
public:
	OrNode(const Position * p, ExpNode * lhs_in, ExpNode * rhs_in): BinaryExpNode(p, lhs_in, rhs_in){};
	void unparse(std::ostream& out, int indent);
};

class PlusNode : public BinaryExpNode{
public:
	PlusNode(const Position * p, ExpNode * lhs_in, ExpNode * rhs_in): BinaryExpNode(p, lhs_in, rhs_in){};
	void unparse(std::ostream& out, int indent);
};

class TimesNode : public BinaryExpNode{
public:
	TimesNode(const Position * p, ExpNode * lhs_in, ExpNode * rhs_in): BinaryExpNode(p, lhs_in, rhs_in){};
	void unparse(std::ostream& out, int indent);
};


} //End namespace drewno_mars

#endif
