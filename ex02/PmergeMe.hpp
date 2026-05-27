#ifndef PMERGEME_HPP
#define PMERGEME_HPP

#include <iostream>
#include <vector>
#include <deque>
#include <string>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include <cctype>
#include <ctime>
#include <utility>

class PmergeMe
{
	private:
		std::vector<int> _vector;
		std::deque<int> _deque;

		double _vectorTime;
		double _dequeTime;

		size_t _vectorComparisons;
		size_t _dequeComparisons;

	public:
		PmergeMe();
		PmergeMe(const PmergeMe& other);
		PmergeMe& operator=(const PmergeMe& other);
		~PmergeMe();

		void parseInput(int ac, char **av);
		void sort();
		void displayBefore() const;
		void displayAfter() const;
		void displayTime() const;
};

#endif
