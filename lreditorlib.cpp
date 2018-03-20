#include "lrlib.h"

std::pair<std::string, std::vector<std::string>> readRule(const std::string& line, size_t ruleIndex, std::pair<std::string, std::vector<std::string>>& mainRule)
{
	std::pair<std::string, std::vector<std::string>> rule;
	auto begin = line.begin();
	if (begin == line.end()) return rule;
	auto lhs = *begin;
	++begin;
	std::vector<std::string> rhs;
	while (begin != line.end()) {
		if (*begin == '\'') {++begin; continue;}
		std::string temp;
		temp.push_back(*begin);
		if (*begin != ' ') rhs.push_back(temp);
		++begin;		
	}	
	rule.first = lhs;
	rule.second = rhs;	
	if (ruleIndex == 0) mainRule = rule;
	return rule;
}

Grammar readGrammar(const std::string& grammar_file_name, grammar_rule& mainR)
{
	Grammar grammar;
	std::string line;
	std::ifstream grammar_file(grammar_file_name);
	if (!grammar_file) return grammar;
	size_t ruleIndex = 0;
	std::pair<std::string, std::vector<std::string>> mainRule;
	while (std::getline(grammar_file, line))  
	{
		auto rule = readRule(line, ruleIndex, mainRule);
		if (ruleIndex == 0) rule.second.push_back("$");
		grammar.insert(rule);
		++ruleIndex;
	}
	dumpGrammar(grammar);
	mainR.lhs = mainRule.first;
	mainR.rhs = mainRule.second;
	mainR.rhs.push_back("$");
	std::cout << "mainRule : " << mainR.lhs << std::endl;
	return grammar;
}

std::string readInput(const std::string& input_file_name)
{
	std::ifstream input_file(input_file_name);
	if (!input_file) return "";
	std::istream_iterator<char> begin(input_file);
	std::istream_iterator<char> end;
	std::string input;
	while (begin != end) {
		if (*begin == '.') break;
		input.push_back(*begin);
		++begin;
	}
	input.push_back('$');
	return input;
}



int main()
{	
	
	grammar_rule mainRule(" ", {});
	auto grammar = readGrammar("grammar.txt", mainRule);
	auto input = readInput("input.txt");
	
	StateCollection stateCollection;
	auto CC = generateLR0Items(grammar, mainRule, stateCollection);
	auto parseTable = generateActionTable(stateCollection, CC, grammar);
	dumpActionTable(parseTable);
	dumpGrammar(grammar);
	std::string output;
	parse_node_ptr tree;
	std::string stackOutput;
	bool error = parseTree(input, parseTable, grammar, tree, stackOutput);
	(void)error;
	std::string visitOutput;
	visit(tree, visitOutput);
	std::ofstream out("parseTree.txt");
	out << "Stack (Token,State): [";
	size_t index = 0;
	for (const auto& elem : stackOutput) {
		if (index % 2 == 0) {
			out << "(";
			out << elem;
			out << ",";
		}
		else {
			out << elem;
			out << ")";
		}				
		++index;
	}
	out << "]";
	out << '\n';
	out << visitOutput;
	
	std::ofstream stackOut("stack.txt");
	stackOut << stackOutput;
	return 0;
}
