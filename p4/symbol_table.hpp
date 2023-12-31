#ifndef DREWNO_MARS_SYMBOL_TABLE_HPP
#define DREWNO_MARS_SYMBOL_TABLE_HPP
#include <string>
#include <unordered_map>
#include <list>

//Use an alias template so that we can use
// "HashMap" and it means "std::unordered_map"
template <typename K, typename V>
using HashMap = std::unordered_map<K, V>;

using namespace std;

namespace drewno_mars{

//A semantic symbol, which represents a single
// variable, function, etc. Semantic symbols 
// exist for the lifetime of a scope in the 
// symbol table. 
class SemSymbol {
	//TODO add the fields that 
	// each semantic symbol should track
	// (i.e. the kind of the symbol (either a variable or function)
	// and functions to get/set those fields
	public:
		SemSymbol(std::string kind, std::string type, std::string name)
		: myKind(kind), myType(type), myName(name) {};
		std::string getName(){return myName;};
		std::string getType(){return myType;};
	private:
		std::string myKind;
		std::string myType;
		std::string myName;
};

//A single scope. The symbol table is broken down into a 
// chain of scope tables, and each scope table holds 
// semantic symbols for a single scope. For example,
// the globals scope will be represented by a ScopeTable,
// and the contents of each function can be represented by
// a ScopeTable.
class ScopeTable {
	public:
		ScopeTable();
		//TODO: add functions for looking up symbols
		// and/or returning information to indicate
		// that the symbol does not exist within the
		// current scope.
		bool insertEntry(std::string symName, SemSymbol * sym);
		SemSymbol * findEntry(std::string symName);
	private:
		HashMap<std::string, SemSymbol *> * symbols;
};

class SymbolTable{
	public:
		SymbolTable();
		//TODO: add functions to create a new ScopeTable
		// when a new scope is entered, drop a ScopeTable
		// when a scope is exited, etc. 
		void insertScope(ScopeTable * scope);
		void remove();
		bool insertSymToScope(SemSymbol * sym);
		SemSymbol * searchScopes(std::string symbol);
	private:
		std::list<ScopeTable *> * scopeTableChain;
};

	
}

#endif
