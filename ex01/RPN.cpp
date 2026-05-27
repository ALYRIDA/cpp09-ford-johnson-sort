#include "RPN.hpp"

RPN::RPN()
{}

RPN::RPN(const RPN& other)
{
	*this = other;
}

RPN &RPN::operator=(const RPN& other)
{
	if (this != &other)
		this->_stack = other._stack;
	return *this;
}

RPN::~RPN()
{}

bool RPN::isOperatorToken(const std::string& token) const
{
	return token == "+" || token == "-" || token == "*" ||token == "/";
}

int RPN::doOperator(int left, int right, const std::string& op) const
{
	if (op == "+")
		return left+right;
	if (op == "-")
		return left-right;
	if (op == "*")
		return left*right;
	if (op == "/")
	{
		if (right == 0)
			throw std::runtime_error("ERROR");
		return left/right;
	}
	throw std::runtime_error("ERROR");
}

int RPN::solve(const std::string& expression)
{
	std::stringstream ss(expression);
	std::string token;
	while (ss >> token)
	{
		if (token.length() == 1 && std::isdigit(token[0]))
			_stack.push(token[0] - '0');// convert to int by '- 0'
		else if (isOperatorToken(token))
		{
			if (_stack.size() < 2)
				throw std::runtime_error("ERROR");
			int right = _stack.top();
			_stack.pop();
			int left = _stack.top();
			_stack.pop();
			int result = doOperator(left, right, token);
			_stack.push(result);
		}
		else
			throw std::runtime_error("ERROR");
	}
	if (_stack.size() != 1)
		throw std::runtime_error("ERROR");
	return _stack.top();
}
