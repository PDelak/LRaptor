#ifndef LANGUAGE_STUDIO_LR_LIBRARY_H
#define LANGUAGE_STUDIO_LR_LIBRARY_H

#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <set>
#include <initializer_list>
#include <list>
#include <cassert>
#include <fstream>
#include <any>
#include <stack>
#include <memory>
#include <fstream>
#include <iterator>
#include <algorithm>

template<typename T>
std::string toString(T value)
{
	std::stringstream sstr;
	sstr << value;
	return sstr.str();
}

typedef std::multimap<std::string, std::vector<std::string>> Grammar;
typedef std::map<std::pair<size_t, std::string>, std::pair<std::string, size_t>> ParseTable;


struct parse_node;
typedef std::shared_ptr<parse_node> parse_node_ptr;

struct parse_node
{
	parse_node() :token(0) {}
	parse_node(const std::string& t) :token(t) {}
	std::string token;
	std::vector<parse_node_ptr> children;
};


void visit(std::string& prefix, const parse_node_ptr& node, std::string& output, bool& root)
{
	if (!node) return;
	for (auto& e : prefix) {
		e = ' ';
	}
	if (!root) prefix = prefix + "|-";
	root = false;
	output += "\n" + prefix + node->token;

	for (const auto& child : node->children) {
		visit(prefix, child, output, root);
		
	}
	if (!prefix.empty()) prefix.pop_back();
	if (!prefix.empty()) prefix.pop_back();
}

void visitBlock(std::string& prefix, const parse_node_ptr& node, std::string& output)
{
	if (!node) return;
	if (node->token == "$") return;
	output += "[ \"" +  node->token + "\" ";

	for (const auto& child : node->children) {
		visitBlock(prefix, child, output);
	}
	output += "]";
}

void visitBlock(const parse_node_ptr& node, std::string& output)
{
	std::string prefix = "";
	visitBlock(prefix, node, output);
}


void visit(const parse_node_ptr& node, std::string& output)
{
	std::string prefix = "";
	bool root = true;
	visit(prefix, node, output, root);
	std::cout << std::endl;
}


struct grammar_rule
{
    grammar_rule(const std::string& l, const std::vector<std::string>& r)
        :lhs(l), rhs(r)
    {}

	grammar_rule(const std::string& l, const std::initializer_list<std::string>& r)
		:lhs(l)
    {
        auto b = r.begin();
        auto e = r.end();
        for (;b != e;++b) {
            rhs.push_back(*b);
        }
    }
    std::string lhs;
    bool operator<(const grammar_rule& r) const { return lhs < r.lhs || (lhs == r.lhs && rhs < r.rhs);}
    bool operator==(const grammar_rule& r) const { return lhs == r.lhs && rhs == r.rhs; }
    std::vector<std::string> rhs;
};

size_t getGrammarRuleIndex(const Grammar& grammar, const grammar_rule& rule)
{    
    auto range = grammar.equal_range(rule.lhs);
    auto begin = range.first;
    for (; begin != range.second; ++begin) {
        if ((*begin).second == rule.rhs) break;
    }
    return std::distance(grammar.begin(), begin);
}

struct item
{
    grammar_rule rule;
    size_t dotPosition;
    bool operator<(const item& i) const { return rule < i.rule || (rule == i.rule && dotPosition < i.dotPosition); }
    bool operator==(const item& i) const { return rule == i.rule && dotPosition == i.dotPosition; }
    item(const grammar_rule& r, size_t p):rule(r), dotPosition(p) {}
};


std::set<item> lr0closure(const std::set<item>& is, const Grammar& grammar)
{
    std::set<item> result;
    std::list<item> openItems;
    
    // push all items from input set 
    // to open one
    for (auto e : is) {
        openItems.push_back(e);
    }

    // go through open set items
    // and add those that meet the criteria
    while (!openItems.empty()) {
        item LR0Item = *openItems.begin();
        result.insert(LR0Item);
        openItems.erase(openItems.begin());
        if (LR0Item.rule.rhs.size() <= LR0Item.dotPosition) continue;
        std::string nextLHSProduction = LR0Item.rule.rhs[LR0Item.dotPosition];
        auto rule_iter = grammar.equal_range(nextLHSProduction);
        for (rule_iter; rule_iter.first != rule_iter.second; ++rule_iter.first) {
            grammar_rule r(rule_iter.first->first, rule_iter.first->second);
            item newItem(r, 0);
            if(std::find(openItems.begin(), openItems.end(), newItem) == openItems.end() && result.find(newItem) == result.end())
                openItems.push_back(newItem);
        }
    }
    return result;
}

std::set<item> lr0goto(const std::set<item>& is, const std::string& token, const Grammar& grammar)
{
    std::set<item> gotoSet;

    // foreach item in input itemset
    for (auto i : is) {
        size_t position = i.dotPosition;
        if (i.rule.rhs.size() <= position) continue;
        if (i.rule.rhs[position] == token) {
            item newItem(i.rule, ++position);            
            gotoSet.insert(newItem);
        }

    }
    return lr0closure(gotoSet, grammar);
}

std::string serializeItemSet(const Grammar& grammar, const std::set<item>& closedSet, size_t id)
{
    std::string output;
    int index = 0;
    std::stringstream ss;
    ss << id;
    output += "State ";
    output += ss.str();
    output += '\n';

    for (auto e : closedSet) {
        output += (e.rule.lhs); 
        output += " -> ";
        for (auto v : e.rule.rhs) {
            if (index > 0) output += " ";
            if (index == e.dotPosition) output += ".";
            output += v;
            ++index;
        }

        index = 0;
        if (e.dotPosition == e.rule.rhs.size()) output += ".";
        output += '\n';
    }
    return output;
}


void showItemSet(const Grammar& grammar, const std::set<item>& closedSet)
{
    std::cout << serializeItemSet(grammar, closedSet, 0).c_str();
}

std::set<std::string> tokens(const Grammar& grammar)
{
    std::set<std::string> tokens;
    for (auto rule : grammar) {
        tokens.insert(rule.first);
        for (auto rhs : rule.second) {
            tokens.insert(rhs);
        }
    }
    return tokens;
}

struct Edge
{
    Edge(size_t f, size_t t, const std::string& tk):from(f), to(t), token(tk) {}
    size_t from;
    size_t to;
    std::string token;
    bool operator < (const Edge& e) const {
        return (from < e.from) || (from == e.from && to < e.to) || (from == e.from && to == e.to && token < e.token);
    }
    bool operator == (const Edge& e) const {
        return from == e.from && to == e.to && token == e.token;
    }
};

struct StateCollection
{
    StateCollection():setId(0) {}
    
    void addItemSet(const std::set<item>& itemSet)
    {
        itemSetIds[itemSet] = getCurrentStateId();
        idToItemSet[getCurrentStateId()] = itemSet;
        getNextStateId();
    }

    bool itemSetExists(size_t id) const
    {
        auto it = idToItemSet.find(id);
        return (it != idToItemSet.end());        
    }

    bool itemSetExists(const std::set<item>& itemSet) const
    {
        auto it = itemSetIds.find(itemSet);
        return (it != itemSetIds.end());
    }

    size_t getItemSetId(const std::set<item>& itemSet) const
    {
        auto it = itemSetIds.find(itemSet);
        if (it == itemSetIds.end()) throw 1;
        return it->second;
    }

    std::set<item> getItemSetById(size_t id) const
    {
        auto it = idToItemSet.find(id);
        if (it == idToItemSet.end()) throw 1;
        return it->second;
    }

    size_t getNumberOfStates() const { return itemSetIds.size(); }

private:
    size_t getNextStateId() { ++setId; return setId; }
    size_t getCurrentStateId() const { return setId; }
    std::map<std::set<item>, size_t> itemSetIds;
    std::map<size_t, std::set<item>> idToItemSet;

    size_t setId;
};

std::set<Edge> generateLR0Items(const Grammar& grammar, const grammar_rule& mainRule, StateCollection& stateCollection)
{
    typedef std::set<item> ItemSet;
    std::set<Edge> gotoEdges;
    ItemSet initialSet;
    item initialItem(mainRule, 0);
    initialSet.insert(initialItem);
    auto startItemSet = lr0closure(initialSet, grammar);

    stateCollection.addItemSet(startItemSet);

    std::list<ItemSet> open;
    open.push_back(startItemSet);
    auto tokenSet = tokens(grammar);

    while (!open.empty()) {
        auto begin = open.begin();
        auto itemSet = *begin;
        open.erase(begin);
        for (auto t :tokenSet) {
            auto gotoSet = lr0goto(itemSet, t, grammar);
            if (gotoSet.empty()) continue;

            if (!stateCollection.itemSetExists(gotoSet)) {
                stateCollection.addItemSet(gotoSet);
                open.push_back(gotoSet);
            }
            auto from = stateCollection.getItemSetId(itemSet);
            auto to = stateCollection.getItemSetId(gotoSet);
            Edge edge(from, to, t);
            gotoEdges.insert(edge);
        }
    }
    return gotoEdges;
}

std::set<std::string> terminals(const Grammar& grammar)
{
    std::set<std::string> terminalsSet;
    auto allTokens = tokens(grammar);
    std::set<std::string> nonTerminals;
    for (auto rule : grammar) {
        nonTerminals.insert(rule.first);
    }
    for (auto token : allTokens) {
        if (nonTerminals.find(token) == nonTerminals.end()) {
            terminalsSet.insert(token);
        }
    }
    return terminalsSet;
}

std::set<std::string> nonTerminals(const Grammar& grammar)
{
    std::set<std::string> nonTerminalsSet;
    for (auto rule : grammar) {
        nonTerminalsSet.insert(rule.first);
    }
    return nonTerminalsSet;
}

ParseTable generateActionTable(const StateCollection& stateCollection, const std::set<Edge>& lr0CanonicalCollection, const Grammar& grammar)
{    
    ParseTable actionTable;
    auto terms = terminals(grammar);
    // fill shifts
    for (auto edge : lr0CanonicalCollection) {
        std::cout << "from : " << edge.from << " to : " << edge.to << " token : " << edge.token << std::endl;
        
        std::string action = "g";
        if (terms.find(edge.token) != terms.end()) {
            action = "s";
        }
        actionTable[std::make_pair(edge.from, edge.token)] = std::make_pair(action, edge.to);
    }
    // fill reductions for final states

    auto stateNum = stateCollection.getNumberOfStates();
    for (size_t i = 0; i < stateNum; ++i) {
        auto itemSet = stateCollection.getItemSetById(i);
        if (itemSet.size() != 1) continue;
        auto item = *itemSet.begin();
        if (item.dotPosition == item.rule.rhs.size()) {
            size_t ruleNumber = getGrammarRuleIndex(grammar, item.rule);            
            std::string action = "r";
            if (item.rule.rhs[item.dotPosition-1] == "$") action = "a";			
            for (const auto token : terms) {
                actionTable[std::make_pair(i, token)] = std::make_pair(action, ruleNumber);
            }
        }
    }

    return actionTable;
}

bool parse(const std::string& input, const ParseTable& parseTable, const Grammar& grammar, std::string& output)
{
	std::vector<std::any> parseStack;
	parseStack.push_back(std::make_any<size_t>(0));
	auto tokenPtr = input.begin();
	std::string token;
	bool error = false;
	if (tokenPtr == input.end()) return error;
	
	for (;;) {
		auto state = std::any_cast<size_t>(*parseStack.rbegin());
		
		if (tokenPtr != input.end()) token = *tokenPtr;
		auto actionIterator = parseTable.find(std::make_pair(state, token));
		if (actionIterator == parseTable.end()) { error = true; break; }

		auto action = actionIterator->second.first;	
		if (action == "s") {
			parseStack.push_back(std::make_any<std::string>(token));
			auto newState = actionIterator->second.second;
			parseStack.push_back(std::make_any<size_t>(newState));
			++tokenPtr;
		}
		else if (action == "r") {
			size_t ruleIndex = actionIterator->second.second;
			auto ruleIterator = grammar.begin();
			std::advance(ruleIterator , ruleIndex);
			auto rhsSize = ruleIterator->second.size();
			for(size_t i = 0; i < rhsSize * 2; ++i) parseStack.pop_back();
			auto newState = std::any_cast<size_t>(*parseStack.rbegin());
			output.append(ruleIterator->first);
			parseStack.push_back(std::make_any<std::string>(ruleIterator->first));
			actionIterator = parseTable.find(std::make_pair(newState, ruleIterator->first));
			auto state = actionIterator->second.second;
			parseStack.push_back(std::make_any<size_t>(state));
		}
		else if (action == "a") {
			size_t ruleIndex = actionIterator->second.second;
			auto ruleIterator = grammar.begin();
			std::advance(ruleIterator, ruleIndex);
			auto rhsSize = ruleIterator->second.size();
			for (size_t i = 0; i < rhsSize * 2; ++i) parseStack.pop_back();
			auto newState = std::any_cast<size_t>(*parseStack.rbegin());
			output.append(ruleIterator->first);
			parseStack.push_back(std::make_any<std::string>(ruleIterator->first));		
			break;
		}
		else {
			error = true;
			break;
		}

	}
	return error;
}

bool parseTree(const std::string& input, const ParseTable& parseTable, const Grammar& grammar, parse_node_ptr& tree, std::vector<std::string>& stackOutput)
{
	std::vector<std::any> parseStack;
	parseStack.push_back(std::make_any<size_t>(0));
	auto tokenPtr = input.begin();
	std::string token;
	bool error = false;
	if (tokenPtr == input.end()) return error;
	parse_node_ptr root;
	for (;;) {
		auto state = std::any_cast<size_t>(*parseStack.rbegin());

		if (tokenPtr != input.end()) token = *tokenPtr;		
		auto actionIterator = parseTable.find(std::make_pair(state, token));
		if (actionIterator == parseTable.end()) { error = true; break; }

		auto action = actionIterator->second.first;
		if (action == "s") {
			auto node = parse_node_ptr(new parse_node(token));
			parseStack.push_back(std::make_any<parse_node_ptr>(node));
			auto newState = actionIterator->second.second;
			parseStack.push_back(std::make_any<size_t>(newState));
			++tokenPtr;
			stackOutput.push_back(node->token);
			stackOutput.push_back(toString(newState));
		}
		else if (action == "r") {
			size_t ruleIndex = actionIterator->second.second;
			auto ruleIterator = grammar.begin();
			std::advance(ruleIterator, ruleIndex);
			
			auto rhsSize = ruleIterator->second.size();
			auto parent = parse_node_ptr(new parse_node(ruleIterator->first));
			for (size_t i = 0; i < rhsSize * 2; ++i) {
				if (i % 2 != 0) {
					auto node = std::any_cast<parse_node_ptr>(*parseStack.rbegin());
					parent->children.push_back(node);

				}
				parseStack.pop_back();
				stackOutput.pop_back();
			}
			auto currentState = std::any_cast<size_t>(*parseStack.rbegin());
			parseStack.push_back(std::make_any<parse_node_ptr>(parent));
			actionIterator = parseTable.find(std::make_pair(currentState, ruleIterator->first));
			auto state = actionIterator->second.second;
			parseStack.push_back(std::make_any<size_t>(state));
			stackOutput.push_back(parent->token);
			stackOutput.push_back(toString(state));
			root = parent;
		}
		else if (action == "a") {
			size_t ruleIndex = actionIterator->second.second;
			auto ruleIterator = grammar.begin();
			std::advance(ruleIterator, ruleIndex);
			auto rhsSize = ruleIterator->second.size();
			auto parent = parse_node_ptr(new parse_node(ruleIterator->first));
			for (size_t i = 0; i < rhsSize * 2; ++i) {
				if (i % 2 != 0) {
					auto node = std::any_cast<parse_node_ptr>(*parseStack.rbegin());
					parent->children.push_back(node);

				}
				parseStack.pop_back();
				stackOutput.pop_back();

			}
			parseStack.push_back(std::make_any<parse_node_ptr>(parent));
			stackOutput.push_back(parent->token);
			stackOutput.push_back(toString(state));
			root = parent;
			break;
		}
		else {
			error = true;
			break;
		}			
	}
	if (!error) {
			tree = std::any_cast<parse_node_ptr>(*parseStack.rbegin());
	}
	else {
		tree = root;
	}
	
	return error;
}


void dumpActionTable(const ParseTable& actionTable)
{
    for (const auto & key : actionTable) {
        std::cout << "(" << key.first.first << "," << key.first.second << ")" << " = " << key.second.first << key.second.second << std::endl;
    }
}

void dumpGrammar(const Grammar& grammar)
{
    std::string out = "";
    size_t index = 0;
    
    for (const auto& rule : grammar) {
        std::stringstream indexstream;
        indexstream << index;
        indexstream << " : ";        
        out += indexstream.str();
        out += rule.first;
        out += " -> ";
        for (const auto& symbol : rule.second) {
            out += symbol;
            out += " ";
        }
        out += ";\n";
        ++index;
    }
    std::cout << out.c_str() << std::endl;
}

std::string generateDot(const Grammar& grammar, const StateCollection& stateCollection, const std::set<Edge>& CC)
{
    std::string output;
    output += "digraph grammar {\n";
    for (auto edge : CC) {
        
        auto fromItemSet = stateCollection.getItemSetById(edge.from);
        auto toItemSet = stateCollection.getItemSetById(edge.to);
        auto token = edge.token;

        output += "\"";
        output += serializeItemSet(grammar, fromItemSet, edge.from);
        output += "\"\n";
        output += "->\n";
        output += "\"";        
        output += serializeItemSet(grammar, toItemSet, edge.to);
        output += "\"\n";
        output += "[label=";
        output += "\"";
        output += token;
        output += "\"]\n";

    }

    output += "}\n";
    return output;
}

void testLR0closure1(const Grammar& grammar)
{
    std::set<item> initial_set;
    grammar_rule mainRule("E", { "0" });
    item initial_item(mainRule, 0);
    initial_set.insert(initial_item);
    auto closedSet = lr0closure(initial_set, grammar);

    std::set<item> expectedSet;
    grammar_rule rule("E", { "0" });
    item expectedItem(rule, 0);
    expectedSet.insert(expectedItem);
    assert(closedSet == expectedSet);
}

void testLR0closure2(const Grammar& grammar)
{
    std::set<item> initial_set;
    grammar_rule mainRule("M", {"S", "E" });
    item initial_item(mainRule, 1);
    initial_set.insert(initial_item);
    auto closedSet = lr0closure(initial_set, grammar);

    std::set<item> expectedSet;
    grammar_rule rule("M", { "S", "E" });
    item expectedItem(rule, 1);
    expectedSet.insert(expectedItem);
    rule = grammar_rule("E", { "0" });
    expectedItem = item(rule, 0);
    expectedSet.insert(expectedItem);
    rule = grammar_rule("E", { "1" });
    expectedItem = item(rule, 0);
    expectedSet.insert(expectedItem);
    assert(closedSet == expectedSet);
}


void testGrammar(const Grammar& grammar, const grammar_rule& mainRule, const std::string& input)
{
	auto term = terminals(grammar);
	std::cout << "terminals" << std::endl;
	for (const auto& t : term) {
		std::cout << t << std::endl;
	}
	std::cout << "non-terminals" << std::endl;
	for (const auto& t : nonTerminals(grammar)) {
		std::cout << t << std::endl;
	}

	StateCollection stateCollection;
	auto CC = generateLR0Items(grammar, mainRule, stateCollection);
	std::ofstream out("graph.dot");
	out << generateDot(grammar, stateCollection, CC).c_str();
	auto parseTable = generateActionTable(stateCollection, CC, grammar);
	dumpActionTable(parseTable);
	dumpGrammar(grammar);
	std::string output;
	parse_node_ptr tree;
	std::vector<std::string> stackOutput;
	parseTree(input, parseTable, grammar, tree, stackOutput);
	size_t indent = 0;
	std::string visitOutput;
	visit(tree, visitOutput);
	std::cout << visitOutput << std::endl;

}

#endif
