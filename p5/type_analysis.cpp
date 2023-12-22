#include "ast.hpp"
#include "symbol_table.hpp"
#include "errors.hpp"
#include "types.hpp"
#include "name_analysis.hpp"
#include "type_analysis.hpp"

//if we ever want to do this make sure we do 
// (condType->asFn()->getReturnType()->isBool()
// make it 
// condType->asFn() && condType->asFn()->getReturnType()->isBool() 
// to prevent any segfaults

namespace drewno_mars{

TypeAnalysis * TypeAnalysis::build(NameAnalysis * nameAnalysis){
	//To emphasize that type analysis depends on name analysis
	// being complete, a name analysis must be supplied for 
	// type analysis to be performed.
	TypeAnalysis * typeAnalysis = new TypeAnalysis();
	auto ast = nameAnalysis->ast;	
	typeAnalysis->ast = ast;

	ast->typeAnalysis(typeAnalysis);
	if (typeAnalysis->hasError){
		return nullptr;
	}

	return typeAnalysis;
}

void ProgramNode::typeAnalysis(TypeAnalysis * ta){

	//pass the TypeAnalysis down throughout
	// the entire tree, getting the types for
	// each element in turn and adding them
	// to the ta object's hashMap
	for (auto global : *myGlobals){
		global->typeAnalysis(ta);
	}

	//The type of the program node will never
	// be needed. We can just set it to VOID
	//(Alternatively, we could make our type 
	// be error if the DeclListNode is an error)
	ta->nodeType(this, BasicType::produce(VOID));
}

void FnDeclNode::typeAnalysis(TypeAnalysis * ta){

	//HINT: you might want to change the signature for
	// typeAnalysis on FnBodyNode to take a second
	// argument which is the type of the current 
	// function. This will help you to know at a 
	// return statement whether the return type matches
	// the current function

	//Note: this function may need extra code
	IDNode * myID = this->ID();
	SemSymbol * nameSymbol = myID->getSymbol();
	const DataType * nameType = nameSymbol->getDataType();
	const FnType * symbolFn = nameType->asFn();

	ta->setCurrentFnType(symbolFn);

	for (auto stmt : *myBody){
		stmt->typeAnalysis(ta);
	}
}

void StmtNode::typeAnalysis(TypeAnalysis * ta){
	TODO("Implement me in the subclass");
}

void AssignStmtNode::typeAnalysis(TypeAnalysis * ta){
	//TODO: Note that this function is incomplete. 
	// and needs additional code

	//Do typeAnalysis on the subexpressions
	myDst->typeAnalysis(ta);
	mySrc->typeAnalysis(ta);

	const DataType * tgtType = ta->nodeType(myDst);
	const DataType * srcType = ta->nodeType(mySrc);

	// As error returns null if subType is NOT an error type
	// otherwise, it returns the subType itself. It 
	// sort of serves as a way to cast the subtype
	if (tgtType->asError() || srcType->asError()){
		ta->nodeType(this, ErrorType::produce());
		return;
	}

	// While incomplete, this gives you one case for 
	// assignment: if the types are exactly the same
	// it is usually ok to do the assignment. One
	// exception is that if both types are function
	// names, it should fail type analysis

	//target is a function - throw error
	//source is a function(not a function call) - throw error
	if(tgtType->asFn() || ((srcType->asFn()) && !(mySrc->isFnCall()))){
		ta->errAssignOpd(this->pos());
	}
	//source is a function call that matches target type
	if(mySrc->isFnCall() && tgtType == srcType->asFn()->getReturnType()){
		ta->nodeType(this, tgtType);
		return;
	}
	//target and source have the same type
	if (tgtType == srcType){
		ta->nodeType(this, tgtType);
		return;
	}
	
	//Some functions are already defined for you to
	// report type errors. Note that these functions
	// also tell the typeAnalysis object that the
	// analysis has failed, meaning that main.cpp
	// will print "Type check failed" at the end
	ta->errAssignOpr(this->pos());


	// Here, we set the type of the assignment
	// to void to indicate no error was found.
	// This step is optional, since you'll never
	// use the type of a statement
	ta->nodeType(this, BasicType::produce(VOID));
}
//input
//must be int or bool
void TakeStmtNode::typeAnalysis(TypeAnalysis * ta){
	myDst->typeAnalysis(ta);
	const DataType * inType = ta->nodeType(myDst);

	if(inType->asFn() && !(myDst->isFnCall())){
		ta->errReadFn(myDst->pos());
		ta->nodeType(this, ErrorType::produce());
		return;
	} else if(inType->asClass()){
		ta->errReadClass(myDst->pos());
		ta->nodeType(this, ErrorType::produce());
		return;
	}

	if(inType->asBasic()->isBool() || inType->asBasic()->isInt()){
		ta->nodeType(this, ta->nodeType(myDst));
		return;
	}

	ta->nodeType(this, ErrorType::produce());
}
//output
//must be int bool or string
void GiveStmtNode::typeAnalysis(TypeAnalysis * ta){
	mySrc->typeAnalysis(ta);
	const DataType * outType = ta->nodeType(mySrc);
	
	if(outType->asFn() && !(mySrc->isFnCall())){
		ta->errOutputFn(mySrc->pos());
		ta->nodeType(this, ErrorType::produce());
		return;
	} else if(outType->asClass()){
		ta->errOutputClass(mySrc->pos());
		ta->nodeType(this, ErrorType::produce());
		return;
	} else if(outType->asBasic()->isVoid()){
		ta->errOutputVoid(mySrc->pos());
		ta->nodeType(this, ErrorType::produce());
		return;
	}

	if(outType->asBasic()->isBool() || outType->asBasic()->isInt() || outType->asBasic()->isString()){
		ta->nodeType(this, ta->nodeType(mySrc));
		return;
	}

	//nothing should reach this but if it does its an error?
	ta->nodeType(this, ErrorType::produce());
}

void ExitStmtNode::typeAnalysis(TypeAnalysis * ta){
	//this should never not be void, right?
	ta->nodeType(this, BasicType::produce(VOID));
}

void PostDecStmtNode::typeAnalysis(TypeAnalysis * ta){
	myLoc->typeAnalysis(ta);
	const DataType * curType = ta->nodeType(myLoc);
	//not an int so not valid
	if(!curType->isInt()){
		ta->errMathOpd(myLoc->pos());
		ta->nodeType(this, ErrorType::produce());
		return;
	}

	ta->nodeType(this, ta->nodeType(myLoc));
}

void PostIncStmtNode::typeAnalysis(TypeAnalysis * ta){
	myLoc->typeAnalysis(ta);
	const DataType * curType = ta->nodeType(myLoc);
	//not an int so not valid
	if(!curType->isInt()){
		ta->errMathOpd(myLoc->pos());
		ta->nodeType(this, ErrorType::produce());
		return;
	}

	ta->nodeType(this, ta->nodeType(myLoc));
}

void IfStmtNode::typeAnalysis(TypeAnalysis * ta){
	myCond->typeAnalysis(ta);
	const DataType * condType = ta->nodeType(myCond);
	//condition is some kind of bool
	if(condType->isBool()){
		ta->nodeType(this, BasicType::produce(VOID));
		return;
	} else if(condType->asFn() && condType->asFn()->getReturnType()->isBool()){
		ta->nodeType(this, BasicType::produce(VOID));
		return;
	}
	for(auto stmt : *myBody){
		stmt->typeAnalysis(ta);
	}
	//not a bool = error
	ta->errCond(myCond->pos());
	ta->nodeType(this, ErrorType::produce());
}

void IfElseStmtNode::typeAnalysis(TypeAnalysis * ta){
	myCond->typeAnalysis(ta);
	const DataType * condType = ta->nodeType(myCond);
	//condition is some kind of bool
	if(condType->isBool()){
		ta->nodeType(this, BasicType::produce(VOID));
		return;
	} else if(condType->asFn() && condType->asFn()->getReturnType()->isBool()){
		ta->nodeType(this, BasicType::produce(VOID));
		return;
	}
	for(auto stmt : *myBodyTrue){
		stmt->typeAnalysis(ta);
	}
	for(auto stmt : *myBodyFalse){
		stmt->typeAnalysis(ta);
	}
	//not a bool = error
	ta->errCond(myCond->pos());
	ta->nodeType(this, ErrorType::produce());
}

void WhileStmtNode::typeAnalysis(TypeAnalysis * ta){
	myCond->typeAnalysis(ta);
	const DataType * condType = ta->nodeType(myCond);
	//condition is some kind of bool
	if(condType->isBool()){
		ta->nodeType(this, BasicType::produce(VOID));
		return;
	} else if(condType->asFn() && condType->asFn()->getReturnType()->isBool()){
		ta->nodeType(this, BasicType::produce(VOID));
		return;
	}
	for(auto stmt : *myBody){
		stmt->typeAnalysis(ta);
	}
	//not a bool = error
	ta->errCond(myCond->pos());
	ta->nodeType(this, ErrorType::produce());
}

void ReturnStmtNode::typeAnalysis(TypeAnalysis * ta){
	const DataType * retType;
	//check the type of expression being returned
	if(myExp){
		myExp->typeAnalysis(ta);
		retType = ta->nodeType(myExp);
	} else {
		retType = BasicType::produce(VOID);
	}

	if(retType->asError()){
		ta->errRetWrong(myExp->pos());
		ta->nodeType(this, ErrorType::produce());
		return;
	}

	const DataType * fnType = ta->getCurrentFnType();

	//fntype void  &&    ret type not void
	if(fnType->isVoid() && !(retType->isVoid())){
		ta->errRetWrong(myExp->pos());
		ta->nodeType(this, ErrorType::produce());
		return;
	}
	//fntype not void  &&    ret type void
	if(retType->isVoid() && !fnType->isVoid()){
		ta->errRetEmpty(this->pos());
		ta->nodeType(this, ErrorType::produce());
		return;
	}
	//ret type is fn
	if(retType->asFn()){
		if(fnType != (retType->asFn()->getReturnType())){
			ta->errRetWrong(myExp->pos());
			ta->nodeType(this, ErrorType::produce());
			return;
		}
		ta->nodeType(this, ta->nodeType(myExp));
		return;
	}
	//return type n function type dont match
	if(retType != fnType){
		ta->errRetWrong(myExp->pos());
		ta->nodeType(this, ErrorType::produce());
		return;
	}
	//if it is here it has passed all the error tests
	ta->nodeType(this, ta->nodeType(myExp));
}

void CallExpNode::typeAnalysis(TypeAnalysis * ta){
	myCallee->typeAnalysis(ta);
	const DataType * callType = ta->nodeType(myCallee); 
	if(!callType->asFn()){
		ta->errCallee(myCallee->pos());
		ta->nodeType(this, ErrorType::produce());
		return;
	}

	auto formalTypeList = callType->asFn()->getFormalTypes();
	if(formalTypeList->getSize() != myArgs->size()){
		ta->errArgCount(this->pos());
		ta->nodeType(this, ErrorType::produce());
		return;
	}
	auto formalTypes = formalTypeList->getTypes();
	
	auto formalIter = formalTypes->begin();
	for(auto arg : *myArgs){
		if(formalIter == formalTypes->end()) break;
		auto formal = *formalIter;
		arg->typeAnalysis(ta);
		auto argType = ta->nodeType(arg);

		if(formal!=argType){
			ta->errArgMatch(arg->pos());
		}

		formalIter++;
	}

	if(!ta->passed()){
		ta->nodeType(this, ErrorType::produce());
		return;
	}

	ta->nodeType(this, callType);
}
// class--memberofclass
void MemberFieldExpNode::typeAnalysis(TypeAnalysis * ta){
	myBase->typeAnalysis(ta);
	myField->typeAnalysis(ta);
	const DataType * classType = ta->nodeType(myBase);
	const DataType * fieldType = ta->nodeType(myField);

	if(!(classType->asClass())){
		ta->errAssignNonLVal(myBase->pos());
		ta->nodeType(this, ErrorType::produce());
		return;
	}
	//check if field is a member of class
	SemSymbol * myFieldSem = classType->asClass()->getField(classType->asClass(), myField->getName());

	if(myFieldSem == NULL){
		ta->errAssignNonLVal(myField->pos());
		ta->nodeType(this, ErrorType::produce());
		return;
	}

	ta->nodeType(this, fieldType);
}

void ExpNode::typeAnalysis(TypeAnalysis * ta){
	TODO("Override me in the subclass");
}

void DeclNode::typeAnalysis(TypeAnalysis * ta){
	TODO("Override me in the subclass");
}

void VarDeclNode::typeAnalysis(TypeAnalysis * ta){
	// VarDecls always pass type analysis, since they 
	// are never used in an expression. You may choose
	// to type them void (like this), as discussed in class
	ta->nodeType(this, BasicType::produce(VOID));
}

void IDNode::typeAnalysis(TypeAnalysis * ta){
	// IDs never fail type analysis and always
	// yield the type of their symbol (which
	// depends on their definition)
	ta->nodeType(this, this->getSymbol()->getDataType());
}

void IntLitNode::typeAnalysis(TypeAnalysis * ta){
	// IntLits never fail their type analysis and always
	// yield the type INT
	ta->nodeType(this, BasicType::produce(INT));
}

void StrLitNode::typeAnalysis(TypeAnalysis * ta){
	ta->nodeType(this, BasicType::produce(STRING));
}

void TrueNode::typeAnalysis(TypeAnalysis * ta){
	ta->nodeType(this, BasicType::produce(BOOL));
}

void FalseNode::typeAnalysis(TypeAnalysis * ta){
	ta->nodeType(this, BasicType::produce(BOOL));
}

void MagicNode::typeAnalysis(TypeAnalysis * ta){
	ta->nodeType(this, BasicType::produce(BOOL));
}

void PlusNode::typeAnalysis(TypeAnalysis * ta){
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	const DataType * left = ta->nodeType(myExp1);
	const DataType * right = ta->nodeType(myExp2);

	if(left->asFn() == nullptr){
		if(!(left->isInt())){
			ta->errMathOpd(myExp1->pos());
		}
	} else {
		if(!(left->asFn()->getReturnType()->isInt()) || !(myExp1->isFnCall())){
			ta->errMathOpd(myExp1->pos());
		}
	}

	if(right->asFn() == nullptr){
		if(!(right->isInt())){
			ta->errMathOpd(myExp2->pos());
		}
	} else {
		if(!(right->asFn()->getReturnType()->isInt()) || !(myExp2->isFnCall())){
			ta->errMathOpd(myExp2->pos());
		}
	}
	//if errors were thrown, make error type
	if (!ta->passed())
	{
		ta->nodeType(this, ErrorType::produce());
		return;
	}
	ta->nodeType(this, BasicType::produce(INT));
}

void MinusNode::typeAnalysis(TypeAnalysis * ta){
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	const DataType * left = ta->nodeType(myExp1);
	const DataType * right = ta->nodeType(myExp2);

	if(left->asFn() == nullptr){
		if(!(left->isInt())){
			ta->errMathOpd(myExp1->pos());
		}
	} else {
		if(!(left->asFn()->getReturnType()->isInt()) || !(myExp1->isFnCall())){
			ta->errMathOpd(myExp1->pos());
		}
	}

	if(right->asFn() == nullptr){
		if(!(right->isInt())){
			ta->errMathOpd(myExp2->pos());
		}
	} else {
		if(!(right->asFn()->getReturnType()->isInt()) || !(myExp2->isFnCall())){
			ta->errMathOpd(myExp2->pos());
		}
	}
	//if errors were thrown, make error type
	if (!ta->passed())
	{
		ta->nodeType(this, ErrorType::produce());
		return;
	}
	ta->nodeType(this, BasicType::produce(INT));
}

void TimesNode::typeAnalysis(TypeAnalysis * ta){
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	const DataType * left = ta->nodeType(myExp1);
	const DataType * right = ta->nodeType(myExp2);

	if(left->asFn() == nullptr){
		if(!(left->isInt())){
			ta->errMathOpd(myExp1->pos());
		}
	} else {
		if(!(left->asFn()->getReturnType()->isInt()) || !(myExp1->isFnCall())){
			ta->errMathOpd(myExp1->pos());
		}
	}

	if(right->asFn() == nullptr){
		if(!(right->isInt())){
			ta->errMathOpd(myExp2->pos());
		}
	} else {
		if(!(right->asFn()->getReturnType()->isInt()) || !(myExp2->isFnCall())){
			ta->errMathOpd(myExp2->pos());
		}
	}
	//if errors were thrown, make error type
	if (!ta->passed())
	{
		ta->nodeType(this, ErrorType::produce());
		return;
	}
	ta->nodeType(this, BasicType::produce(INT));
}

void DivideNode::typeAnalysis(TypeAnalysis * ta){
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	const DataType * left = ta->nodeType(myExp1);
	const DataType * right = ta->nodeType(myExp2);

	if(left->asFn() == nullptr){
		if(!(left->isInt())){
			ta->errMathOpd(myExp1->pos());
		}
	} else {
		if(!(left->asFn()->getReturnType()->isInt()) || !(myExp1->isFnCall())){
			ta->errMathOpd(myExp1->pos());
		}
	}

	if(right->asFn() == nullptr){
		if(!(right->isInt())){
			ta->errMathOpd(myExp2->pos());
		}
	} else {
		if(!(right->asFn()->getReturnType()->isInt()) || !(myExp2->isFnCall())){
			ta->errMathOpd(myExp2->pos());
		}
	}
	//if errors were thrown, make error type
	if (!ta->passed())
	{
		ta->nodeType(this, ErrorType::produce());
		return;
	}
	ta->nodeType(this, BasicType::produce(INT));
}

void AndNode::typeAnalysis(TypeAnalysis * ta){
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	const DataType * left = ta->nodeType(myExp1);
	const DataType * right = ta->nodeType(myExp2);

	if(left->asFn() == nullptr){
		if(!(left->isBool())){
			ta->errLogicOpd(myExp1->pos());
		}
	} else {
		if(!(left->asFn()->getReturnType()->isBool()) || !(myExp1->isFnCall())){
			ta->errLogicOpd(myExp1->pos());
		}
	}

	if(right->asFn() == nullptr){
		if(!(right->isBool())){
			ta->errLogicOpd(myExp2->pos());
		}
	} else {
		if(!(right->asFn()->getReturnType()->isBool()) || !(myExp2->isFnCall())){
			ta->errLogicOpd(myExp2->pos());
		}
	}
	//if errors were thrown, make error type
	if (!ta->passed())
	{
		ta->nodeType(this, ErrorType::produce());
		return;
	}
	ta->nodeType(this, BasicType::produce(BOOL));
}

void OrNode::typeAnalysis(TypeAnalysis * ta){
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	const DataType * left = ta->nodeType(myExp1);
	const DataType * right = ta->nodeType(myExp2);

	if(left->asFn() == nullptr){
		if(!(left->isBool())){
			ta->errLogicOpd(myExp1->pos());
		}
	} else {
		if(!(left->asFn()->getReturnType()->isBool()) || !(myExp1->isFnCall())){
			ta->errLogicOpd(myExp1->pos());
		}
	}

	if(right->asFn() == nullptr){
		if(!(right->isBool())){
			ta->errLogicOpd(myExp2->pos());
		}
	} else {
		if(!(right->asFn()->getReturnType()->isBool()) || !(myExp2->isFnCall())){
			ta->errLogicOpd(myExp2->pos());
		}
	}
	//if errors were thrown, make error type
	if (!ta->passed())
	{
		ta->nodeType(this, ErrorType::produce());
		return;
	}
	ta->nodeType(this, BasicType::produce(BOOL));
}

void EqualsNode::typeAnalysis(TypeAnalysis * ta){
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	const DataType * left = ta->nodeType(myExp1);
	const DataType * right = ta->nodeType(myExp2);
	//not functions
	if(left->asFn() == nullptr){
		if(right->asFn() == nullptr){
			if(left==right){
				ta->nodeType(this, BasicType::produce(BOOL));
				return;
			} else {
				ta->errEqOpr(this->pos());
			}
		} else {
			if(!(myExp2->isFnCall())){
				ta->errEqOpd(myExp2->pos());
			}
			if(left==(right->asFn()->getReturnType())){
				ta->nodeType(this, BasicType::produce(BOOL));
				return;
			} else {
				ta->errEqOpr(this->pos());
			}
		}
	} else {
		if(right->asFn() == nullptr){
			if(!(myExp1->isFnCall())){
				ta->errEqOpd(myExp1->pos());
			}
			if((left->asFn()->getReturnType()) == right){
				ta->nodeType(this, BasicType::produce(BOOL));
				return;
			} else {
				ta->errEqOpr(this->pos());
			}
		} else {
			if(!(myExp2->isFnCall())){
				ta->errEqOpd(myExp2->pos());
			}
			if((left->asFn()->getReturnType())==(right->asFn()->getReturnType())){
				ta->nodeType(this, BasicType::produce(BOOL));
				return;
			} else {
				ta->errEqOpr(this->pos());
			}
		}
	}
	
	ta->nodeType(this, ErrorType::produce());
}

void NotEqualsNode::typeAnalysis(TypeAnalysis * ta){
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	const DataType * left = ta->nodeType(myExp1);
	const DataType * right = ta->nodeType(myExp2);

	if(left->asFn() == nullptr){
		if(right->asFn() == nullptr){
			if(left==right){
				ta->nodeType(this, BasicType::produce(BOOL));
				return;
			} else {
				ta->errEqOpr(this->pos());
			}
		} else {
			if(!(myExp2->isFnCall())){
				ta->errEqOpd(this->pos());
			}
			if(left==(right->asFn()->getReturnType())){
				ta->nodeType(this, BasicType::produce(BOOL));
				return;
			} else {
				ta->errEqOpr(this->pos());
			}
		}
	} else {
		if(right->asFn() == nullptr){
			if(!(myExp1->isFnCall())){
				ta->errEqOpd(myExp1->pos());
			}
			if((left->asFn()->getReturnType()) == right){
				ta->nodeType(this, BasicType::produce(BOOL));
				return;
			} else {
				ta->errEqOpr(this->pos());
			}
		} else {
			if(!(myExp2->isFnCall())){
				ta->errEqOpd(myExp2->pos());
			}
			if((left->asFn()->getReturnType())==(right->asFn()->getReturnType())){
				ta->nodeType(this, BasicType::produce(BOOL));
				return;
			} else {
				ta->errEqOpr(this->pos());
			}
		}
	}
	
	ta->nodeType(this, ErrorType::produce());
}

void LessNode::typeAnalysis(TypeAnalysis * ta){
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	const DataType * left = ta->nodeType(myExp1);
	const DataType * right = ta->nodeType(myExp2);

	if(left->asFn() == nullptr){
		if(!(left->isBool())){
			ta->errLogicOpd(myExp1->pos());
		}
	} else {
		if(!(left->asFn()->getReturnType()->isBool()) || !(myExp1->isFnCall())){
			ta->errLogicOpd(myExp1->pos());
		}
	}

	if(right->asFn() == nullptr){
		if(!(right->isBool())){
			ta->errLogicOpd(myExp2->pos());
		}
	} else {
		if(!(right->asFn()->getReturnType()->isBool()) || !(myExp2->isFnCall())){
			ta->errLogicOpd(myExp2->pos());
		}
	}
	//if errors were thrown, make error type
	if (!ta->passed())
	{
		ta->nodeType(this, ErrorType::produce());
		return;
	}
	ta->nodeType(this, BasicType::produce(BOOL));
}

void LessEqNode::typeAnalysis(TypeAnalysis * ta){
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	const DataType * left = ta->nodeType(myExp1);
	const DataType * right = ta->nodeType(myExp2);

	if(left->asFn() == nullptr){
		if(!(left->isBool())){
			ta->errLogicOpd(myExp1->pos());
		}
	} else {
		if(!(left->asFn()->getReturnType()->isBool()) || !(myExp1->isFnCall())){
			ta->errLogicOpd(myExp1->pos());
		}
	}

	if(right->asFn() == nullptr){
		if(!(right->isBool())){
			ta->errLogicOpd(myExp2->pos());
		}
	} else {
		if(!(right->asFn()->getReturnType()->isBool()) || !(myExp2->isFnCall())){
			ta->errLogicOpd(myExp2->pos());
		}
	}
	//if errors were thrown, make error type
	if (!ta->passed())
	{
		ta->nodeType(this, ErrorType::produce());
		return;
	}
	ta->nodeType(this, BasicType::produce(BOOL));
}

void GreaterNode::typeAnalysis(TypeAnalysis * ta){
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	const DataType * left = ta->nodeType(myExp1);
	const DataType * right = ta->nodeType(myExp2);

	if(left->asFn() == nullptr){
		if(!(left->isBool())){
			ta->errLogicOpd(myExp1->pos());
		}
	} else {
		if(!(left->asFn()->getReturnType()->isBool()) || !(myExp1->isFnCall())){
			ta->errLogicOpd(myExp1->pos());
		}
	}

	if(right->asFn() == nullptr){
		if(!(right->isBool())){
			ta->errLogicOpd(myExp2->pos());
		}
	} else {
		if(!(right->asFn()->getReturnType()->isBool()) || !(myExp2->isFnCall())){
			ta->errLogicOpd(myExp2->pos());
		}
	}
	//if errors were thrown, make error type
	if (!ta->passed())
	{
		ta->nodeType(this, ErrorType::produce());
		return;
	}
	ta->nodeType(this, BasicType::produce(BOOL));
}

void GreaterEqNode::typeAnalysis(TypeAnalysis * ta){
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	const DataType * left = ta->nodeType(myExp1);
	const DataType * right = ta->nodeType(myExp2);

	if(left->asFn() == nullptr){
		if(!(left->isBool())){
			ta->errLogicOpd(myExp1->pos());
		}
	} else {
		if(!(left->asFn()->getReturnType()->isBool()) || !(myExp1->isFnCall())){
			ta->errLogicOpd(myExp1->pos());
		}
	}

	if(right->asFn() == nullptr){
		if(!(right->isBool())){
			ta->errLogicOpd(myExp2->pos());
		}
	} else {
		if(!(right->asFn()->getReturnType()->isBool()) || !(myExp2->isFnCall())){
			ta->errLogicOpd(myExp2->pos());
		}
	}
	//if errors were thrown, make error type
	if (!ta->passed())
	{
		ta->nodeType(this, ErrorType::produce());
		return;
	}
	ta->nodeType(this, BasicType::produce(BOOL));
}

void NegNode::typeAnalysis(TypeAnalysis * ta){
	myExp->typeAnalysis(ta);
	const DataType * exp = ta->nodeType(myExp);

	if(exp->isInt() || exp->asFn()->getReturnType()->isInt()){
		ta->nodeType(this, exp);
		return;
	}

	ta->errMathOpd(myExp->pos());
	ta->nodeType(this, ErrorType::produce());
}

void NotNode::typeAnalysis(TypeAnalysis * ta){
	myExp->typeAnalysis(ta);
	const DataType * exp = ta->nodeType(myExp);

	if(exp->isInt() || exp->asFn()->getReturnType()->isInt()){
		ta->nodeType(this, exp);
		return;
	}

	ta->errMathOpd(myExp->pos());
	ta->nodeType(this, ErrorType::produce());
}

void CallStmtNode::typeAnalysis(TypeAnalysis * ta){
	myCallExp->typeAnalysis(ta);
	ta->nodeType(this, ta->nodeType(myCallExp));
}

}
