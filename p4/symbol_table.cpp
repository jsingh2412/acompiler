#include "symbol_table.hpp"
#include <iostream>
namespace drewno_mars{

//SemSymbol

//ScopeTable
ScopeTable::ScopeTable(){
	symbols = new HashMap<std::string, SemSymbol *>();
}
bool ScopeTable::insertEntry(std::string symName, SemSymbol * sym){
	std::pair<std::string, SemSymbol*> entry(symName, sym);
	return (symbols->insert(entry).second);
}
SemSymbol * ScopeTable::findEntry(std::string symName){
	std::unordered_map<std::string, SemSymbol *>::const_iterator entry = symbols->find(symName);
	if (entry == symbols->end()) {
		return nullptr;
	}
	return entry->second;
}

//SymbolTable
SymbolTable::SymbolTable(){
	//TODO: implement the list of hashtables approach
	// to building a symbol table:
	// Upon entry to a scope a new scope table will be 
	// entered into the front of the chain and upon exit the 
	// latest scope table will be removed from the front of 
	// the chain.
	scopeTableChain = new std::list<ScopeTable *>();
	//ScopeTable * global = new ScopeTable();
	//insertScope(global);
}
void SymbolTable::insertScope(ScopeTable * scope){
	scopeTableChain->push_back(scope);
}
void SymbolTable::remove(){
	scopeTableChain->pop_back();
}
bool SymbolTable::insertSymToScope(SemSymbol * sym){
	/*ScopeTable * cur = scopeTableChain->back();
	if(cur){
		bool outcome = cur->insertEntry(sym->getName(), sym);
		return outcome;
	}*/
	return scopeTableChain->back()->insertEntry(sym->getName(), sym);
}
SemSymbol * SymbolTable::searchScopes(std::string symbol){
	SemSymbol * temp = nullptr;
	for(auto scope : *scopeTableChain){
		temp = scope->findEntry(symbol);
		if(temp != nullptr){
			return temp;
		}
	}
	return temp;
}
}
