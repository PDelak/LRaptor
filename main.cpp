#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <set>
#include <initializer_list>
#include <list>
#include <cassert>
#include <fstream>

typedef std::multimap<char, std::vector<char>> Grammar;
typedef std::map<std::pair<size_t, char>, std::pair<char, size_t>> ActionTable;
typedef std::map<std::pair<int, char>, int> GotoTable;


struct grammar_rule
{
    grammar_rule(char l, const std::vector<char>& r)
        :lhs(l), rhs(r)
    {}

    grammar_rule(char l, const std::initializer_list<char>& r)
        :lhs(l)
    {
        auto b = r.begin();
        auto e = r.end();
        for (;b != e;++b) {
            rhs.push_back(*b);
        }
    }
    char lhs;
    bool operator<(const grammar_rule& r) const { return lhs < r.lhs || (lhs == r.lhs && rhs < r.rhs);}
    bool operator==(const grammar_rule& r) const { return lhs == r.lhs && rhs == r.rhs; }
    std::vector<char> rhs;
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
        char nextLHSProduction = LR0Item.rule.rhs[LR0Item.dotPosition];
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

std::set<item> lr0goto(const std::set<item>& is, char token, const Grammar& grammar)
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

std::string serializeItemSet(const Grammar& grammar, const std::set<item>& closedSet, int id)
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

std::set<char> tokens(const Grammar& grammar)
{
    std::set<char> tokens;
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
    Edge(size_t f, size_t t, char tk):from(f), to(t), token(tk) {}
    size_t from;
    size_t to;
    char token;
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

std::set<char> terminals(const Grammar& grammar)
{
    std::set<char> terminalsSet;
    auto allTokens = tokens(grammar);
    std::set<char> nonTerminals;
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

std::set<char> nonTerminals(const Grammar& grammar)
{
    std::set<char> nonTerminalsSet;
    for (auto rule : grammar) {
        nonTerminalsSet.insert(rule.first);
    }
    return nonTerminalsSet;
}

ActionTable generateActionTable(const StateCollection& stateCollection, const std::set<Edge>& lr0CanonicalCollection, const Grammar& grammar)
{
    
    ActionTable actionTable;
    auto terms = terminals(grammar);
    // fill shifts
    for (auto edge : lr0CanonicalCollection) {
        std::cout << "from : " << edge.from << " to : " << edge.to << " token : " << edge.token << std::endl;
        
        char action = ' ';
        if (terms.find(edge.token) != terms.end()) {
            action = 's';
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
            char action = 'r';
            for (const auto token : terms) {
                actionTable[std::make_pair(i, token)] = std::make_pair(action, ruleNumber);
            }
        }
    }

    return actionTable;
}

void dumpActionTable(const ActionTable& actionTable)
{
    for (const auto & key : actionTable) {
        std::cout << "(" << key.first.first << "," << key.first.second << ")" << " = " << key.second.first << key.second.second << std::endl;
    }
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
    grammar_rule mainRule('E', { '0' });
    item initial_item(mainRule, 0);
    initial_set.insert(initial_item);
    auto closedSet = lr0closure(initial_set, grammar);

    std::set<item> expectedSet;
    grammar_rule rule('E', { '0' });
    item expectedItem(rule, 0);
    expectedSet.insert(expectedItem);
    assert(closedSet == expectedSet);
}

void testLR0closure2(const Grammar& grammar)
{
    std::set<item> initial_set;
    grammar_rule mainRule('M', {'S', 'E' });
    item initial_item(mainRule, 1);
    initial_set.insert(initial_item);
    auto closedSet = lr0closure(initial_set, grammar);

    std::set<item> expectedSet;
    grammar_rule rule('M', { 'S', 'E' });
    item expectedItem(rule, 1);
    expectedSet.insert(expectedItem);
    rule = grammar_rule('E', { '0' });
    expectedItem = item(rule, 0);
    expectedSet.insert(expectedItem);
    rule = grammar_rule('E', { '1' });
    expectedItem = item(rule, 0);
    expectedSet.insert(expectedItem);
    assert(closedSet == expectedSet);
}

void testLR0closure3()
{
    /*
    (0) S -> E eof
    (1) E -> E * B
    (2) E -> E + B
    (3) E -> B
    (4) B -> 0
    (5) B -> 1
    */

    Grammar grammar = {
        { 'S',{ 'E' } },
        { 'E',{ 'E', '*', 'B'} },
        { 'E',{ 'E', '+', 'B' } },
        { 'E',{ 'B' } },
        { 'B',{ '0' } },
        { 'B',{ '1' } }
    };
    std::set<item> initial_set;
    grammar_rule mainRule('S', { 'E' });
    item initial_item(mainRule, 0);
    initial_set.insert(initial_item);
    auto closedSet = lr0closure(initial_set, grammar);
    std::cout << "closedSet" << std::endl;
    showItemSet(grammar, closedSet);
    auto gotoSet = lr0goto(closedSet, 'E', grammar);
    std::cout << "gotoSet" << std::endl;
    showItemSet(grammar, gotoSet);

    StateCollection stateCollection;

    auto CC = generateLR0Items(grammar, mainRule, stateCollection);
    for (auto edge : CC) {
        std::cout << "from : " << edge.from << " to : " << edge.to << " token : " << edge.token << std::endl;
    }

    std::ofstream out("graph.dot");
    out << generateDot(grammar, stateCollection, CC).c_str();
}

void testLR0closure4()
{
    /*
    (0) S -> E eof
    (1) E -> E * B
    (2) E -> E + B
    (3) E -> B
    (4) B -> 0
    (5) B -> 1
    */

    Grammar grammar = {
        { 'S',{ 'E' } },
        { 'E',{ 'E', '+', 'E' } },
        { 'E',{ '1' } },
    };
    std::set<item> initial_set;
    grammar_rule mainRule('S', { 'E' });
    item initial_item(mainRule, 0);
    initial_set.insert(initial_item);
    auto closedSet = lr0closure(initial_set, grammar);
    std::cout << "closedSet" << std::endl;
    showItemSet(grammar, closedSet);
    auto gotoSet = lr0goto(closedSet, 'E', grammar);
    std::cout << "gotoSet" << std::endl;
    showItemSet(grammar, gotoSet);

    StateCollection stateCollection;

    auto CC = generateLR0Items(grammar, mainRule, stateCollection);
    for (auto edge : CC) {
        std::cout << "from : " << edge.from << " to : " << edge.to << " token : " << edge.token << std::endl;
    }

    std::ofstream out("graph.dot");
    out << generateDot(grammar, stateCollection, CC).c_str();
}


/*
Z -> E$ E -> T | E ’ + ’ T
T -> i
*/


void testExprGrammar()
{
    Grammar grammar = {
        { 'Z',{ 'E', '$'} },
        { 'E',{ 'T'} },
        { 'E',{ 'E', '+', 'T' } },
        { 'T',{ '1' } },
        { 'T',{ '(', 'E', ')'} } 
    };
    
    std::set<item> initial_set;
    grammar_rule mainRule('Z', { 'E', '$' });
    item initial_item(mainRule, 0);
    initial_set.insert(initial_item);
    auto closedSet = lr0closure(initial_set, grammar);
    std::cout << "closedSet" << std::endl;
    showItemSet(grammar, closedSet);
    auto gotoSet = lr0goto(closedSet, 'E', grammar);
    std::cout << "gotoSet" << std::endl;
    showItemSet(grammar, gotoSet);

    StateCollection stateCollection;

    auto CC = generateLR0Items(grammar, mainRule, stateCollection);
    for (auto edge : CC) {
        std::cout << "from : " << edge.from << " to : " << edge.to << " token : " << edge.token << std::endl;
    }

    std::ofstream out("graph.dot");
    out << generateDot(grammar, stateCollection, CC).c_str();
}

void testExprGrammar2()
{
    Grammar grammar = {
        { 'Z',{ 'E', '$' } },
        { 'E',{ '{', 'T' , '}' } },
        { 'T',{ '1' } }
    };

    std::set<item> initial_set;
    grammar_rule mainRule('Z', { 'E', '$' });
    item initial_item(mainRule, 0);
    initial_set.insert(initial_item);
    auto closedSet = lr0closure(initial_set, grammar);
    std::cout << "closedSet" << std::endl;
    showItemSet(grammar, closedSet);
    auto gotoSet = lr0goto(closedSet, 'E', grammar);
    std::cout << "gotoSet" << std::endl;
    showItemSet(grammar, gotoSet);

    StateCollection stateCollection;

    auto CC = generateLR0Items(grammar, mainRule, stateCollection);
    for (auto edge : CC) {
        std::cout << "from : " << edge.from << " to : " << edge.to << " token : " << edge.token << std::endl;
    }

    std::ofstream out("graph.dot");
    out << generateDot(grammar, stateCollection, CC).c_str();
}

void testExprGrammar3()
{
    /*
    Grammar grammar = {
        { 'Z',{ 'E', '$' } },
        { 'E',{ '{', 'T' , '}' } },
        { 'T',{ '1' } }
    };
    */
    Grammar grammar = {
        { 'Z',{ 'E', '$' } },
        { 'E',{ 'T' } },
        { 'E',{ 'E', '+', 'T' } },
        { 'T',{ '1' } },
        { 'T',{ '(', 'E', ')' } }
    };
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
    grammar_rule mainRule('Z', { 'E', '$' });
    auto CC = generateLR0Items(grammar, mainRule, stateCollection);
    auto actionTable = generateActionTable(stateCollection, CC, grammar);
    dumpActionTable(actionTable);

}

int main()
{
    testExprGrammar();
    testExprGrammar3();
    /*
    Grammar grammar = {
        {'M',{'S', 'E'}},
        {'S',{'E'}},
        {'E',{'0'}},
        {'E',{'1'}}
    };
    
    testLR0closure1(grammar);
    testLR0closure2(grammar);
    
    testLR0closure4();
    */
    return 0;
}
