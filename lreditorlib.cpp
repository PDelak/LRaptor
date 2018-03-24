#include "lrlib.h"

struct scanning_state
{
	scanning_state(const std::string& input) :current(input.begin()), end(input.end()) {}
	std::string::const_iterator current;
	std::string::const_iterator end;
	bool eof() const { return current == end; }
};

bool isQuotationMark(char c)
{
	return c == '\'';
}



std::string getNextToken(scanning_state& state)
{
	if (!state.eof() && isspace(*state.current)) {
		auto end = std::find_if_not(state.current, state.end, isspace);
		state.current = end;
	}
	if (!state.eof() && isalpha(*state.current)) {
		auto token_end = std::find_if_not(state.current, state.end, isalnum);
		std::string token = std::string(state.current, token_end);
		state.current = token_end;

		return token;
	}
	if (!state.eof() && isdigit(*state.current)) {
		auto token_end = std::find_if_not(state.current, state.end, isdigit);
		auto current = state.current;
		state.current = token_end;
		return std::string(current, token_end);						
	}

	if (!state.eof() && isQuotationMark(*state.current)) {
		auto token_end = std::find_if(state.current + 1, state.end, isQuotationMark);
		std::string token = std::string(state.current, token_end + 1);
		state.current = token_end + 1;
		return token;
	}
	return "";
}

std::pair<std::string, std::vector<std::string>> readRule0(const std::string& line, size_t ruleIndex, std::pair<std::string, std::vector<std::string>>& mainRule)
{
	std::pair<std::string, std::vector<std::string>> rule;
	auto begin = line.begin();
	if (begin == line.end()) return rule;
	auto lhs = *begin;
	++begin;
	std::vector<std::string> rhs;
	while (begin != line.end()) {
		if (*begin == '\'') { ++begin; continue; }
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

std::pair<std::string, std::vector<std::string>> readRule(const std::string& line, size_t ruleIndex, std::pair<std::string, std::vector<std::string>>& mainRule)
{
	std::pair<std::string, std::vector<std::string>> rule;
	auto begin = line.begin();
	if (begin == line.end()) return rule;
	scanning_state scan_state(line);

	auto token = getNextToken(scan_state);
	auto lhs = token;
	std::vector<std::string> rhs;
	while (!token.empty()) {
		token = getNextToken(scan_state);		
		if (token.empty()) break;
		if(token[0] == '\'') {
			token.erase(token.begin());			
			token.pop_back();
			for (auto character : token) {
				std::string temp;
				temp.push_back(character);
				rhs.push_back(temp);
			}

		} else rhs.push_back(token);
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
	std::vector<std::string> stackOutput;
	bool error = parseTree(input, parseTable, grammar, tree, stackOutput);
	(void)error;
	std::string visitOutput;
	visit(tree, visitOutput);
	std::ofstream out("parseTree.txt");
	out << "Stack (Token, State)\n";
	size_t index = 0;
	for (const auto& elem : stackOutput) {
		if (index % 2 == 0) {
			out << "(";
			out << elem;
			out << ",";
		}
		else {
			out << elem;
			out << ")\n";
		}				
		++index;
	}	
	out << '\n';
	out << visitOutput;
	
	std::ofstream stackOut("stack.txt");
	for (const auto& element : stackOutput) {
		stackOut << element;
	}
	std::ofstream blockOut("block.txt");
	std::string blockOutput;
	visitBlock(tree, blockOutput);
	blockOut << blockOutput;
	return 0;
}
