#include "ast.hpp"
#include "symbol_table.hpp"
#include "errors.hpp"

namespace drewno_mars{

//TODO here is a subset of the nodes needed to do nameAnalysis, 
// you should add the rest to allow for a complete treatment
// of any AST

bool ASTNode::nameAnalysis(SymbolTable * symTab){
	throw new ToDoError("This function should have"
		" been overriden in the subclass!");
}
bool ProgramNode::nameAnalysis(SymbolTable * symTab){
	bool res = true;
	ScopeTable * newTable = new ScopeTable();
	symTab->insertScope(newTable);
	for (auto global : *myGlobals){
		res = global->nameAnalysis(symTab) && res;
	}
	symTab->remove();
	return res;
}

bool VarDeclNode::nameAnalysis(SymbolTable * symTab){
	bool nameAnalysisOk = true;
	bool tempAnalysisOk = true;
	if((myType->getType())=="void"){
		std::cerr << "FATAL " << myID->posStr() << ": Invalid type in declaration.\n";
		return false;
	}

	SemSymbol * checkDup = symTab->searchScopes(myID->getName());
	if(checkDup!=nullptr){
		std::cerr << "FATAL " << myID->posStr() << ": Multiply declared identifier.\n";
		return false;
	}
	
	SemSymbol * varDecl = new SemSymbol(std::string("var"), myType->getType(), myID->getName());
	myID->attachSymbol(varDecl);
	symTab->insertSymToScope(varDecl);
	if(myInit){
		tempAnalysisOk= myInit->nameAnalysis(symTab);
		if(nameAnalysisOk && !tempAnalysisOk){
			nameAnalysisOk = tempAnalysisOk;
		}
	}
	return nameAnalysisOk;
}

bool FnDeclNode::nameAnalysis(SymbolTable * symTab){
	bool nameAnalysisOk = true;
	bool tempAnalysisOk = true;
	std::string type = "(";
	std::string comma = "";
	for(auto formal : *myFormals){
		type.append(comma + formal->getTypeNode()->getType());
		comma = ",";
	}
	type.append(")");
	type.append("->" + myRetType->getType());
	SemSymbol * fnDecl = new SemSymbol(std::string("fn"), type, myID->getName());

	SemSymbol * checkDup = symTab->searchScopes(myID->getName());
	if(checkDup!=nullptr){
		std::cerr << "FATAL " << myID->posStr() << ": Multiply declared identifier.\n";
		nameAnalysisOk = false;
	}

	myID->attachSymbol(fnDecl);
	nameAnalysisOk = symTab->insertSymToScope(fnDecl);
	//add scope
	ScopeTable * fnScope = new ScopeTable();
	symTab->insertScope(fnScope);

	for(auto formal : *myFormals) {
		tempAnalysisOk = formal->nameAnalysis(symTab);
		if(nameAnalysisOk && !tempAnalysisOk){
			nameAnalysisOk = tempAnalysisOk;
		}
	}
	for(auto stmt : *myBody){
		tempAnalysisOk = stmt->nameAnalysis(symTab);
		if(nameAnalysisOk && !tempAnalysisOk){
			nameAnalysisOk = tempAnalysisOk;
		}
	}
	//remove scope
	symTab->remove();
	
	return nameAnalysisOk;
}
bool AssignStmtNode::nameAnalysis(SymbolTable* symTab){
	bool nameAnalysisOk = myDst->nameAnalysis(symTab);
	if(nameAnalysisOk){
		nameAnalysisOk = mySrc->nameAnalysis(symTab);
	}
	return nameAnalysisOk;
}
bool IDNode::nameAnalysis(SymbolTable* symTab){
	SemSymbol * sym = symTab->searchScopes(name);
	if(sym==nullptr){
		std::cerr << "FATAL " << myPos->span() << ": Undeclared identifier.\n";
		return false;
	}
	attachSymbol(sym);
	return true;
}
bool ClassDefnNode::nameAnalysis(SymbolTable* symTab){
	bool nameAnalysisOk;
	SemSymbol * classDecl = new SemSymbol(std::string("var"), myID->getName(), myID->getName());
	myID->attachSymbol(classDecl);
	nameAnalysisOk = symTab->insertSymToScope(classDecl);
	ScopeTable* scope = new ScopeTable();
	symTab->insertScope(scope);
	if(nameAnalysisOk){
		for(auto member : *myMembers){
			nameAnalysisOk = member->nameAnalysis(symTab);
			if(!nameAnalysisOk){
				symTab->remove();
				std::cout<<"WHAT\n";
				return false;
			}
		}
		symTab->remove();
		return true;
	}std::cout<<"OUT\n";
	return false;
}
bool PostDecStmtNode::nameAnalysis(SymbolTable* symTab){
	return myLoc->nameAnalysis(symTab);
}
bool PostIncStmtNode::nameAnalysis(SymbolTable* symTab){
	return myLoc->nameAnalysis(symTab);
}
bool IfStmtNode::nameAnalysis(SymbolTable* symTab){
	if(myCond->nameAnalysis(symTab)){
		ScopeTable * ifScope = new ScopeTable();
		symTab->insertScope(ifScope);
		for(auto stmt : *myBody){
			if(!(stmt->nameAnalysis(symTab))){
				symTab->remove();
				return false;
			}
		}
		symTab->remove();
		return true;
	}
	return false;
}
bool IfElseStmtNode::nameAnalysis(SymbolTable* symTab){
	if(myCond->nameAnalysis(symTab)){
		ScopeTable * ifScope = new ScopeTable();
		symTab->insertScope(ifScope);
		for(auto stmt : *myBodyTrue){
			if(!(stmt->nameAnalysis(symTab))){
				symTab->remove();
				return false;
			}
		}
		symTab->remove();
		ScopeTable * elseScope = new ScopeTable();
		symTab->insertScope(elseScope);
		for(auto stmt : *myBodyFalse){
			if(!(stmt->nameAnalysis(symTab))){
				symTab->remove();
				return false;
			}
		}
		
		symTab->remove();
		return true;
	}
	return false;
}
bool WhileStmtNode::nameAnalysis(SymbolTable* symTab){
	bool nameAnalysisOk = myCond->nameAnalysis(symTab);
	ScopeTable * whileScope = new ScopeTable();
	symTab->insertScope(whileScope);
	//std::cout<<"BEFORELOOP\n";
	for(auto stmt : *myBody){
		//std::cout<<"INLOOP\n";
		nameAnalysisOk = stmt->nameAnalysis(symTab);
	}
	//std::cout<<"ATEND\n";
	symTab->remove();
	return nameAnalysisOk;
}
bool CallExpNode::nameAnalysis(SymbolTable* symTab){
	if(myCallee->nameAnalysis(symTab)){
		for(auto args : *myArgs){
			/*if(args->nameAnalysis(symTab)){
				std::cout<<"THIS IS FAILING123?\n";
				return false;
			}*/
			args->nameAnalysis(symTab);
		}
		return true;
	}
	return false;
}
bool BinaryExpNode::nameAnalysis(SymbolTable* symTab){
	bool first = myExp1->nameAnalysis(symTab);
	bool second = myExp2->nameAnalysis(symTab);
	return (first && second);
}
bool UnaryExpNode::nameAnalysis(SymbolTable* symTab){
	return myExp->nameAnalysis(symTab);
}	
bool CallStmtNode::nameAnalysis(SymbolTable* symTab){
	return myCallExp->nameAnalysis(symTab);
}
bool MemberFieldExpNode::nameAnalysis(SymbolTable* symTab){
	bool first = myBase->nameAnalysis(symTab);
	bool second = myField->nameAnalysis(symTab);
	return (first && second);
}
bool TakeStmtNode::nameAnalysis(SymbolTable* symTab){
	return myDst->nameAnalysis(symTab);
}
bool GiveStmtNode::nameAnalysis(SymbolTable* symTab){
	return mySrc->nameAnalysis(symTab);
}
bool ReturnStmtNode::nameAnalysis(SymbolTable* symTab){
	return myExp->nameAnalysis(symTab);
}
bool IntTypeNode::nameAnalysis(SymbolTable* symTab){
	// Name analysis may never even recurse down to IntTypeNode,
	// but if it does, just return true to indicate that 
	// name analysis has not failed, and add nothing to the symbol table
	return true;
}
bool VoidTypeNode::nameAnalysis(SymbolTable* symTab){
	return true;
}
bool BoolTypeNode::nameAnalysis(SymbolTable* symTab){
	return true;
}
bool PerfectTypeNode::nameAnalysis(SymbolTable* symTab){
	return mySub->nameAnalysis(symTab);
}
bool ClassTypeNode::nameAnalysis(SymbolTable* symTab){
	return myID->nameAnalysis(symTab);
}
bool IntLitNode::nameAnalysis(SymbolTable* symTab){
	return true;
}
bool StrLitNode::nameAnalysis(SymbolTable* symTab){
	return true;
}
bool MagicNode::nameAnalysis(SymbolTable* symTab){
	return true;
}
bool TrueNode::nameAnalysis(SymbolTable* symTab){
	return true;
}
bool FalseNode::nameAnalysis(SymbolTable* symTab){
	return true;
}
bool ExitStmtNode::nameAnalysis(SymbolTable* symTab){
	return true;
}
bool FormalDeclNode::nameAnalysis(SymbolTable* symTab){
	if((myType->getType())=="void"){
		std::cerr << "FATAL " << myPos->span() << ": Invalid type in declaration.\n";
		return false;
	}

	SemSymbol * formalDecl = new SemSymbol(std::string("var"), myType->getType(), myID->getName());
	myID->attachSymbol(formalDecl);
	if(!(symTab->insertSymToScope(formalDecl))){
		std::cerr << "FATAL " << myPos->span() << ": Multiply declared identifier.\n";
		return false;
	}
	return true;
}
}
