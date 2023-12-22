#include <ostream>
#include "3ac.hpp"

namespace drewno_mars{

void IRProgram::allocGlobals(){
	//Choose a label for each global
	for(auto itr: globals){
		SemSymbol * semSymbol = itr.first;
		SymOpd * globalOpd = itr.second;
		std::string lbl = "gbl_" + semSymbol->getName();
		globalOpd->setMemoryLoc(lbl);
	}
	/*
	size_t strNum = 0;
	for(auto str: strings){
		std::string lbl = "\0";
		lbl = "str_" + strNum;
		LitOpd * opd = str.first;
		strNum++;
	}*/
}

void IRProgram::datagenX64(std::ostream& out){
	out << ".globl main\n";
	out << ".data\n";
	for(auto itr: globals){
		if(itr.first->getName()=="console"){continue;}
		SymOpd * globalOpd = itr.second;
		out << globalOpd->getMemoryLoc() << ": .quad 0\n";
	}

	for(auto itr: strings){
		LitOpd * myStr = itr.first;
		std::string myContent = itr.second;
		out << myStr->valString() + ": .asciz " << myContent << "\n";
		out << ".align 8\n";
	}
	//Put this directive after you write out strings
	// so that everything is aligned to a quadword value
	// again
	//out << ".align 8\n";
}

void IRProgram::toX64(std::ostream& out){
	allocGlobals();
	datagenX64(out);
	// Iterate over each procedure and codegen it
	out<<".text\n";
	for(auto p : *procs){
		p->toX64(out);
	}
}

void Procedure::allocLocals(){
	//Allocate space for locals
	// Iterate over each procedure and codegen it
	size_t offset = 16;
	for(auto itr: formals){
		SymOpd * localOpd = itr;
		offset += localOpd->getWidth();
		std::string lbl = "-" + to_string(offset) + "(%rbp)";
		localOpd->setMemoryLoc(lbl);
	}
	for(auto itr: locals){
		SymOpd * localOpd = itr.second;
		offset += localOpd->getWidth();
		std::string lbl = "-" + to_string(offset) + "(%rbp)";
		localOpd->setMemoryLoc(lbl);
	}
	for(auto itr: temps){
		AuxOpd * localOpd = itr;
		offset += localOpd->getWidth();
		std::string lbl = "-" + to_string(offset) + "(%rbp)";
		localOpd->setMemoryLoc(lbl);
	}
	
}

void Procedure::toX64(std::ostream& out){
	//Allocate all locals
	allocLocals();
	out << "#Fn prologue " << myName << "\n";
	enter->codegenLabels(out);
	enter->codegenX64(out);
	out << "#Fn body " << myName << "\n";
	for (auto quad : *bodyQuads){
		quad->codegenLabels(out);
		out << "#" << quad->toString() << "\n";
		quad->codegenX64(out);
	}
	out << "#Fn epilogue " << myName << "\n";
	leave->codegenLabels(out);
	leave->codegenX64(out);
}

void Quad::codegenLabels(std::ostream& out){
	if (labels.empty()){ return; }

	size_t numLabels = labels.size();
	size_t labelIdx = 0;
	for ( Label * label : labels){
		out << label->getName() << ": ";
		if (labelIdx != numLabels - 1){ out << "\n"; }
		labelIdx++;
	}
}

void BinOpQuad::codegenX64(std::ostream& out){
	src1->genLoadVal(out, A);
	src2->genLoadVal(out, B);
	bool genStore = true;
	switch(opr){
		case ADD64:
			out << "addq " << src2->getReg(B) << ", " << src1->getReg(A) << "\n";
			break;
		case ADD8:
			out << "addb " << src2->getReg(B) << ", " << src1->getReg(A) << "\n";
			break;
		case SUB64:
			out << "subq " << src2->getReg(B) << ", " << src1->getReg(A) << "\n";
			break;
		case SUB8:
			out << "subb " << src2->getReg(B) << ", " << src1->getReg(A) << "\n";
			break;
		case MULT64:
			out << "imul " << src2->getReg(B) << ", " << src1->getReg(A) << "\n";
			break;
		case MULT8:
			out << "imulb " << src2->getReg(B) << ", " << src1->getReg(A) << "\n";
			break;
		case DIV64:
			out << "movq $0, %rdx\n";
			out << "idivq %rbx\n";
			break;
		case DIV8:
			out << "movb $0, %rdx\n";
			out << "idivb %rbx\n";
			break;
		case EQ64:
			out << "movq $0, %rcx\n";
			out << "cmpq %rbx, %rax\n";
			out << "sete %cl\n";
			dst->genStoreVal(out, C);
			genStore = false;
			break;
		case EQ8:
			out << "movb $0, %rcx\n";
			out << "cmpb %rbx, %rax\n";
			out << "sete %cl\n";
			dst->genStoreVal(out, C);
			genStore = false;
			break;
		case NEQ64:
			out << "movq $0, %rcx\n";
			out << "cmpq %rbx, %rax\n";
			out << "setne %cl\n";
			dst->genStoreVal(out, C);
			genStore = false;
			break;
		case NEQ8:
			out << "movb $0, %rcx\n";
			out << "cmpb %rbx, %rax\n";
			out << "setne %cl\n";
			dst->genStoreVal(out, C);
			genStore = false;
			break;
		case LT64:
			out << "movq $0, %rcx\n";
			out << "cmpq %rbx, %rax\n";
			out << "setl %cl\n";
			dst->genStoreVal(out, C);
			genStore = false;
			break;
		case LT8:
			out << "movb $0, %rcx\n";
			out << "cmpb %rbx, %rax\n";
			out << "setl %cl\n";
			dst->genStoreVal(out, C);
			genStore = false;
			break;
		case GT64:
			out << "movq $0, %rcx\n";
			out << "cmpq %rbx, %rax\n";
			out << "setg %cl\n";
			dst->genStoreVal(out, C);
			genStore = false;
			break;
		case GT8:
			out << "movb $0, %rcx\n";
			out << "cmpb %rbx, %rax\n";
			out << "setg %cl\n";
			dst->genStoreVal(out, C);
			genStore = false;
			break;
		case LTE64:
			out << "movq $0, %rcx\n";
			out << "cmpq %rbx, %rax\n";
			out << "setle %cl\n";
			dst->genStoreVal(out, C);
			genStore = false;
			break;
		case LTE8:
			out << "movb $0, %rcx\n";
			out << "cmpb %rbx, %rax\n";
			out << "setle %cl\n";
			dst->genStoreVal(out, C);
			genStore = false;
			break;
		case GTE64:
			out << "movq $0, %rcx\n";
			out << "cmpq %rbx, %rax\n";
			out << "setge %cl\n";
			dst->genStoreVal(out, C);
			genStore = false;
			break;
		case GTE8:
			out << "movb $0, %rcx\n";
			out << "cmpb %rbx, %rax\n";
			out << "setge %cl\n";
			dst->genStoreVal(out, C);
			genStore = false;
			break;
		case OR64:
			out << "orq " << src2->getReg(B) << ", " << src1->getReg(A) << "\n";
			break;
		case OR8:
			out << "orb " << src2->getReg(B) << ", " << src1->getReg(A) << "\n";
			break;
		case AND64:
			out << "andq " << src2->getReg(B) << ", " << src1->getReg(A) << "\n";
			break;
		case AND8:
			out << "andb " << src2->getReg(B) << ", " << src1->getReg(A) << "\n";
			break;
	}
	if(genStore){
		dst->genStoreVal(out, A);
	}
}

void UnaryOpQuad::codegenX64(std::ostream& out){
	src->genLoadVal(out,A);
	switch(op){
		case NOT64:
			out<<"notq " << src->getReg(A) << "\n";
			break;
		case NOT8:
			out<<"notb " << src->getReg(A) << "\n";
			break;
		case NEG64:
			out<<"neg " << src->getReg(A) << "\n";
			break;
		case NEG8:
			out<<"neg " << src->getReg(A) << "\n";
			break;
	}
	dst->genStoreVal(out, A);
}

void AssignQuad::codegenX64(std::ostream& out){
	src->genLoadVal(out, A);
	dst->genStoreVal(out, A);
}
//accept int bool
void ReadQuad::codegenX64(std::ostream& out){
	if(myDstType->isInt()){
		out<<"callq getInt\n";
		myDst->genStoreVal(out, A);
	} else if(myDstType){
		out << "callq getBool\n";
		myDst->genStoreVal(out, A);
	} else {
		out << "Why are you reading something illegal.\n";
	}
}

void MagicQuad::codegenX64(std::ostream& out){
	myDst->genLoadVal(out, A);
	out << "callq magic\n";
}

void ExitQuad::codegenX64(std::ostream& out){
	out << "retq\n";
}
//accept int bool string
void WriteQuad::codegenX64(std::ostream& out){	
	if(mySrcType->isBool()){
		mySrc->genLoadVal(out, DI);
		out << "callq printBool\n";
	} else if (mySrcType->isString()){
		mySrc->genLoadVal(out, DI);
		out << "callq printString\n";
	} else if (mySrcType->isInt()){
		mySrc->genLoadVal(out, DI);
		out << "callq printInt\n";
	} else {
		out << "Why are you writing something illegal.\n";
	}
}

void GotoQuad::codegenX64(std::ostream& out){
	out << "jmp " << tgt->getName() << "\n";
}

void IfzQuad::codegenX64(std::ostream& out){
	cnd->genLoadVal(out, A);
	out << "cmpq $0, %rax\n";
	out << "je " << tgt->getName() << "\n";
}

void NopQuad::codegenX64(std::ostream& out){
	out << "nop" << "\n";
}

void CallQuad::codegenX64(std::ostream& out){
	out << "callq fun_" << sym->getName() << "\n";
}

void EnterQuad::codegenX64(std::ostream& out){
	out << "pushq %rbp\n";
	out << "movq %rsp, %rbp\n";
	out << "addq $16, %rbp\n";
	out << "subq $" << myProc->arSize() << ", %rsp\n";
}

void LeaveQuad::codegenX64(std::ostream& out){
	out << "addq $" << myProc->arSize() << ", %rsp\n";
	out << "popq %rbp\n";
	out << "retq\n";
}

void SetArgQuad::codegenX64(std::ostream& out){
	switch(index){
		case 1: opd->genLoadVal(out, DI); break;
		case 2: opd->genLoadVal(out, SI); break;
		case 3: opd->genLoadVal(out, D); break;
		case 4: opd->genLoadVal(out, C); break;
		case 5: opd->genLoadVal(out, R8); break;
		case 6: opd->genLoadVal(out, R9); break;
		default:
			out << "pushq " << opd->getMemoryLoc();
			break;
	}

}

void GetArgQuad::codegenX64(std::ostream& out){
	
}

void SetRetQuad::codegenX64(std::ostream& out){
	opd->genLoadVal(out, A);
}

void GetRetQuad::codegenX64(std::ostream& out){
	out << "retq\n";
}

void LocQuad::codegenX64(std::ostream& out){
	src->genLoadVal(out, A);
	tgt->genStoreVal(out, A);
}

void SymOpd::genLoadVal(std::ostream& out, Register reg){
	out << getMovOp() << " " << getMemoryLoc() << ", " << getReg(reg) << "\n";
}

void SymOpd::genStoreVal(std::ostream& out, Register reg){
	out << getMovOp() << " " <<getReg(reg) << ", " << getMemoryLoc() << "\n";
}

void SymOpd::genLoadAddr(std::ostream& out, Register reg) {
	TODO(Implement me if necessary)
}

void AuxOpd::genLoadVal(std::ostream& out, Register reg){
	out << getMovOp() << " " << getMemoryLoc() << ", " << getReg(reg) << "\n";
}

void AuxOpd::genStoreVal(std::ostream& out, Register reg){
	out << getMovOp() << " " << getReg(reg) << ", " << getMemoryLoc() << "\n";
}
void AuxOpd::genLoadAddr(std::ostream& out, Register reg){
	out << getMovOp() << " " << getMemoryLoc() << ", " << getReg(reg) << "\n";
}


void AddrOpd::genStoreVal(std::ostream& out, Register reg){
	out << getMovOp() << " " << getReg(reg) << ", " << getMemoryLoc() << "\n";
}

void AddrOpd::genLoadVal(std::ostream& out, Register reg){
	out << " " << getMovOp() << " " << getReg(reg) << ", " << getMemoryLoc() << "\n";
}

void AddrOpd::genStoreAddr(std::ostream& out, Register reg){
	out << getMovOp() << " " << getReg(reg) << ", " << getMemoryLoc() << "\n";
}

void AddrOpd::genLoadAddr(std::ostream & out, Register reg){
	out << getMovOp() << " " << getMemoryLoc() << ", " << getReg(reg) << "\n";
}

void LitOpd::genLoadVal(std::ostream & out, Register reg){
	out << getMovOp() << " $" << val << ", " << getReg(reg) << "\n";
}

}
