#include "ast.hpp"

namespace drewno_mars{

IRProgram * ProgramNode::to3AC(TypeAnalysis * ta){
	IRProgram * prog = new IRProgram(ta);
	for (auto global : *myGlobals){
		global->to3AC(prog);
	}
	return prog;
}

void FnDeclNode::to3AC(IRProgram * prog){
	Procedure * proc = prog->makeProc(ID()->getName());
	prog->gatherGlobal(myID->getSymbol());
	for(auto formal: *myFormals){
		formal->to3AC(proc);
	}
	for(auto body: *myBody){
		body->to3AC(proc);
	}
}

void FnDeclNode::to3AC(Procedure * proc){
	//This never needs to be implemented,
	// the function only exists because of
	// inheritance needs (A function declaration
	// never occurs within another function)
	throw new InternalError("FnDecl at a local scope");
}

void FormalDeclNode::to3AC(IRProgram * prog){
	//This never needs to be implemented,
	// the function only exists because of
	// inheritance needs (A formal never
	// occurs at global scope)
	throw new InternalError("Formal at a global scope");
}

void FormalDeclNode::to3AC(Procedure * proc){
	SemSymbol * sym = ID()->getSymbol();
	proc->gatherFormal(sym);
	size_t index = proc->getFormals().size();
	SymOpd* formal = proc->getFormal(index-1);
	//index, opd
	GetArgQuad * arg = new GetArgQuad(index, formal);
	proc->addQuad(arg);
}

Opd * IntLitNode::flatten(Procedure * proc){
	const DataType * type = proc->getProg()->nodeType(this);
	return new LitOpd(std::to_string(myNum), 8);
}

Opd * StrLitNode::flatten(Procedure * proc){
	Opd * res = proc->getProg()->makeString(myStr);
	return res;
}

Opd * TrueNode::flatten(Procedure * proc){
	return new LitOpd(std::to_string(1), 8);
}

Opd * FalseNode::flatten(Procedure * proc){
	return new LitOpd(std::to_string(0), 8);
}

Opd * CallExpNode::flatten(Procedure * proc){
	Opd* callee = myCallee->flatten(proc);
	size_t i = 1;
	for(auto arg: *myArgs){
		Opd* temp = arg->flatten(proc);
		SetArgQuad* quad = new SetArgQuad(i, temp);
		proc->addQuad(quad);
		i++;
	}
	SemSymbol* sym = myCallee->getSymbol();
	Quad* call = new CallQuad(sym);
	proc->addQuad(call);
	Opd* tmp = proc->makeTmp(8);
	GetRetQuad* getRet = new GetRetQuad(tmp);
	proc->addQuad(getRet);
	return callee;
}

Opd * NegNode::flatten(Procedure * proc){
	Opd* exp = myExp->flatten(proc);
	Opd* tmp = proc->makeTmp(8);
	Quad * res = new UnaryOpQuad(tmp, NEG64, exp);
	proc->addQuad(res);
	return tmp;
}

Opd * NotNode::flatten(Procedure * proc){
	Opd* exp = myExp->flatten(proc);
	Opd* tmp = proc->makeTmp(8);
	Quad * res = new UnaryOpQuad(tmp, NOT64, exp);
	proc->addQuad(res);
	return tmp;
}

Opd * PlusNode::flatten(Procedure * proc){
	Opd * lhs = myExp1->flatten(proc);
	Opd * rhs = myExp2->flatten(proc);
	Opd * tmp = proc->makeTmp(8);
	Quad * res = new BinOpQuad(tmp, BinOp::ADD64, lhs, rhs);
	proc->addQuad(res);
	return tmp;
}

Opd * MinusNode::flatten(Procedure * proc){
	Opd * lhs = myExp1->flatten(proc);
	Opd * rhs = myExp2->flatten(proc);
	Opd * tmp = proc->makeTmp(8);
	Quad * res = new BinOpQuad(tmp, BinOp::SUB64, lhs, rhs);
	proc->addQuad(res);
	return tmp;
}

Opd * TimesNode::flatten(Procedure * proc){
	Opd * lhs = myExp1->flatten(proc);
	Opd * rhs = myExp2->flatten(proc);
	Opd * tmp = proc->makeTmp(8);
	Quad * res = new BinOpQuad(tmp, BinOp::MULT64, lhs, rhs);
	proc->addQuad(res);
	return tmp;
}

Opd * DivideNode::flatten(Procedure * proc){
	Opd * lhs = myExp1->flatten(proc);
	Opd * rhs = myExp2->flatten(proc);
	Opd * tmp = proc->makeTmp(8);
	Quad * res = new BinOpQuad(tmp, BinOp::DIV64, lhs, rhs);
	proc->addQuad(res);
	return tmp;
}

Opd * AndNode::flatten(Procedure * proc){
	Opd * lhs = myExp1->flatten(proc);
	Opd * rhs = myExp2->flatten(proc);
	Opd * tmp = proc->makeTmp(8);
	Quad * res = new BinOpQuad(tmp, BinOp::AND64, lhs, rhs);
	proc->addQuad(res);
	return tmp;
}

Opd * OrNode::flatten(Procedure * proc){
	Opd * lhs = myExp1->flatten(proc);
	Opd * rhs = myExp2->flatten(proc);
	Opd * tmp = proc->makeTmp(8);
	Quad * res = new BinOpQuad(tmp, BinOp::OR64, lhs, rhs);
	proc->addQuad(res);
	return tmp;
}

Opd * EqualsNode::flatten(Procedure * proc){
	Opd * lhs = myExp1->flatten(proc);
	Opd * rhs = myExp2->flatten(proc);
	Opd * tmp = proc->makeTmp(8);
	Quad * res = new BinOpQuad(tmp, BinOp::EQ64, lhs, rhs);
	proc->addQuad(res);
	return tmp;
}

Opd * NotEqualsNode::flatten(Procedure * proc){
	Opd * lhs = myExp1->flatten(proc);
	Opd * rhs = myExp2->flatten(proc);
	Opd * tmp = proc->makeTmp(8);
	Quad * res = new BinOpQuad(tmp, BinOp::NEQ64, lhs, rhs);
	proc->addQuad(res);
	return tmp;
}

Opd * LessNode::flatten(Procedure * proc){
	Opd * lhs = myExp1->flatten(proc);
	Opd * rhs = myExp2->flatten(proc);
	Opd * tmp = proc->makeTmp(8);
	Quad * res = new BinOpQuad(tmp, BinOp::LT64, lhs, rhs);
	proc->addQuad(res);
	return tmp;
}

Opd * GreaterNode::flatten(Procedure * proc){
	Opd * lhs = myExp1->flatten(proc);
	Opd * rhs = myExp2->flatten(proc);
	Opd * tmp = proc->makeTmp(8);
	Quad * res = new BinOpQuad(tmp, BinOp::GT64, lhs, rhs);
	proc->addQuad(res);
	return tmp;
}

Opd * LessEqNode::flatten(Procedure * proc){
	Opd * lhs = myExp1->flatten(proc);
	Opd * rhs = myExp2->flatten(proc);
	Opd * tmp = proc->makeTmp(8);
	Quad * res = new BinOpQuad(tmp, BinOp::LTE64, lhs, rhs);
	proc->addQuad(res);
	return tmp;
}

Opd * GreaterEqNode::flatten(Procedure * proc){
	Opd * lhs = myExp1->flatten(proc);
	Opd * rhs = myExp2->flatten(proc);
	Opd * tmp = proc->makeTmp(8);
	Quad * res = new BinOpQuad(tmp, BinOp::GTE64, lhs, rhs);
	proc->addQuad(res);
	return tmp;
}

void AssignStmtNode::to3AC(Procedure * proc){
	Opd* dst = myDst->flatten(proc);
	Opd* src = mySrc->flatten(proc);
	AssignQuad* assign = new AssignQuad(dst, src);
	proc->addQuad(assign);
}

void PostIncStmtNode::to3AC(Procedure * proc){
	Opd* loc = myLoc->flatten(proc);
	LitOpd* one = new LitOpd("1", 8);
	Quad * res = new BinOpQuad(loc, BinOp::ADD64, loc, one);
	proc->addQuad(res);
}

void PostDecStmtNode::to3AC(Procedure * proc){
	Opd* loc = myLoc->flatten(proc);
	LitOpd* one = new LitOpd("1", 8);
	Quad * res = new BinOpQuad(loc, BinOp::SUB64, loc, one);
	proc->addQuad(res);
}

void GiveStmtNode::to3AC(Procedure * proc){
	Opd* src = mySrc->flatten(proc);
	const DataType* type =proc->getProg()->nodeType(mySrc);
	WriteQuad* write = new WriteQuad(src, type);
	proc->addQuad(write);
}

void TakeStmtNode::to3AC(Procedure * proc){
	Opd* dst = myDst->flatten(proc);
	const DataType* type =proc->getProg()->nodeType(myDst);
	ReadQuad* read = new ReadQuad(dst, type);
	proc->addQuad(read);
}

void IfStmtNode::to3AC(Procedure * proc){
	Label* skip = proc->makeLabel();
	Opd* cond = myCond->flatten(proc);
	//create ifzero quad with condition and our created label
	IfzQuad * ifZeroQuad = new IfzQuad(cond, skip);
	proc->addQuad(ifZeroQuad);
	//iterate body
	for(auto stmt : *myBody){
		stmt->to3AC(proc);
	}
	NopQuad* nop = new NopQuad();
	nop->addLabel(skip);
	proc->addQuad(nop);
}

void IfElseStmtNode::to3AC(Procedure * proc){
	Label* skip = proc->makeLabel();
	Label* end = proc->makeLabel();
	Opd* cond = myCond->flatten(proc);
	//create ifzero quad with condition and our created label
	IfzQuad * ifZeroQuad = new IfzQuad(cond, skip);
	proc->addQuad(ifZeroQuad);
	//entering if statement
	for(auto stmt : *myBodyTrue){
		stmt->to3AC(proc);
	}
	//we skip to the end(past else)
	GotoQuad* goTo = new GotoQuad(end);
	proc->addQuad(goTo);
	//entering else statement
	NopQuad* nop = new NopQuad();
	nop->addLabel(skip);
	proc->addQuad(nop);
	for(auto stmt : *myBodyFalse){
		stmt->to3AC(proc);
	}
	NopQuad* secondNop = new NopQuad();
	secondNop->addLabel(end);
	proc->addQuad(secondNop);
}

void WhileStmtNode::to3AC(Procedure * proc){
	Label* start = proc->makeLabel();
	Label* end = proc->makeLabel();
	NopQuad* nop = new NopQuad();
	nop->addLabel(start);
	proc->addQuad(nop);
	Opd* cond = myCond->flatten(proc);
	//while condition
	IfzQuad * ifZeroQuad = new IfzQuad(cond, end);
	proc->addQuad(ifZeroQuad);
	//while body
	for(auto stmt : *myBody){
		stmt->to3AC(proc);
	}
	//loop
	GotoQuad* goTo = new GotoQuad(start);
	proc->addQuad(goTo);
	NopQuad* secondNop = new NopQuad();
	secondNop->addLabel(end);
	proc->addQuad(secondNop);
}

void CallStmtNode::to3AC(Procedure * proc){
	myCallExp->flatten(proc);
	proc->popQuad();
}

void ReturnStmtNode::to3AC(Procedure * proc){
	if(myExp!=nullptr){
		SetRetQuad * retQuad = new SetRetQuad(myExp->flatten(proc));
		proc->addQuad(retQuad);
	}
	proc->addQuad((new GotoQuad(proc->getLeaveLabel())));
}

void ExitStmtNode::to3AC(Procedure * proc){
	proc->addQuad((new GotoQuad(proc->getLeaveLabel())));
}

void VarDeclNode::to3AC(Procedure * proc){
	SemSymbol * sym = ID()->getSymbol();
	assert(sym != nullptr);
	proc->gatherLocal(sym);
}

void VarDeclNode::to3AC(IRProgram * prog){
	SemSymbol * sym = ID()->getSymbol();
	assert(sym != nullptr);
	prog->gatherGlobal(sym);
}

//We only get to this node if we are in a stmt
// context (DeclNodes protect descent)
Opd * IDNode::flatten(Procedure * proc){
	return proc->getSymOpd(getSymbol());
}

}
