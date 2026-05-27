#ifndef RPN_HPP
#define RPN_HPP

#include <iostream>
#include <string>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <cstdlib>

class RPN
{
	private:
		std::stack<int> _stack;
		bool isOperatorToken(const std::string& token) const;
		int doOperator(int left, int right, const std::string& op) const;
	public:
		RPN();
		RPN(const RPN& other);
		RPN &operator=(const RPN& other);
		~RPN();
		int solve(const std::string& expression);
};

#endif