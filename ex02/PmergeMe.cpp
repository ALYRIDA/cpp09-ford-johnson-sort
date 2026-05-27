#include "PmergeMe.hpp"

struct Element
{
	int value;
	size_t id;
};

struct PairNode
{
	Element small;
	Element big;
};

struct PendNode
{
	Element value;
	Element bound;
	bool hasBound;
};

template <typename Seq>
struct PendContainer;

template <typename T, typename Alloc>
struct PendContainer<std::vector<T, Alloc> >
{
	typedef std::vector<PendNode> type;
};

template <typename T, typename Alloc>
struct PendContainer<std::deque<T, Alloc> >
{
	typedef std::deque<PendNode> type;
};

PmergeMe::PmergeMe(): _vectorTime(0), _dequeTime(0), _vectorComparisons(0), _dequeComparisons(0)
{}

PmergeMe::PmergeMe(const PmergeMe& other)
{
	*this = other;
}

PmergeMe& PmergeMe::operator=(const PmergeMe& other)
{
	if (this != &other)
	{
		_vector = other._vector;
		_deque = other._deque;
		_vectorTime = other._vectorTime;
		_dequeTime= other._dequeTime;
		_vectorComparisons = other._vectorComparisons;
		_dequeComparisons = other._dequeComparisons;
	}
	return *this;
}

PmergeMe::~PmergeMe()
{}

static bool isPositiveInteger(const std::string& str)
{
	if (str.empty())
		return false;

	size_t i = 0;
	if (str[0] == '+')
		i = 1;
	if (i >= str.length())
		return false;

	while (i < str.length())
	{
		if (!std::isdigit(str[i]))
			return false;
		i++;
	}
	return true;
}

void PmergeMe::parseInput(int argc, char **argv)
{
	if (argc < 2)
		throw std::runtime_error("Error");
	for (int i = 1; i < argc; i++)
	{
		std::string str = argv[i];
		if (!isPositiveInteger(str))
			throw std::runtime_error("Error");
		long value = std::atol(str.c_str());
		if (value < 0 || value > 2147483647L)
			throw std::runtime_error("Error");
		_vector.push_back(static_cast<int>(value));
		_deque.push_back(static_cast<int>(value));
	}
}

template <typename Seq>
static Seq makeElementSequence(const Seq& input)
{
	Seq result;
	for (size_t i = 0; i < input.size(); i++)
	{
		Element e;
		e.value = input[i];
		e.id = i;
		result.push_back(e);
	}
	return result;
}

template <typename Seq>
static Seq extractValues(const Seq& input)
{
	Seq result;
	for (size_t i = 0; i < input.size(); i++)
		result.push_back(input[i]);
	return result;
}

static std::vector<size_t> jacobsthalOrder(size_t pendSize)
{
	std::vector<size_t> order;

	if (pendSize <= 1)
		return order;
	std::vector<size_t> jacob;
	jacob.push_back(0);
	jacob.push_back(1);
	jacob.push_back(1);
	while (jacob.back() < pendSize)
	{
		size_t n = jacob.size();
		jacob.push_back(jacob[n - 1] + 2 * jacob[n - 2]);
	}
	size_t previous =1;
	for (size_t i = 3; i < jacob.size(); i++)
	{
		size_t current = jacob[i];

		if (current > pendSize)
			current = pendSize;
		for (size_t j =current; j > previous; j--)
			order.push_back(j - 1);
		if (jacob[i] >= pendSize)
			break;
		previous = jacob[i];
	}
	return order;
}

template <typename Seq>
static size_t findBoundPosition(const Seq& main, const Element& bound)
{
	for (size_t i = 0; i < main.size(); i++)
	{
		if (main[i].id == bound.id)
			return i;
	}
	return main.size();
}

static size_t worstBinaryComparisons(size_t rangeSize)
{
	size_t comparisons = 0;
	size_t capacity = 1;

	while (capacity < rangeSize + 1)
	{
		capacity *= 2;
		comparisons++;
	}
	return comparisons;
}

template <typename Seq>
static size_t binaryInsertPosition(
	const Seq& main,
	const Element& value,
	const PendNode& pend,
	size_t& comparisons
)
{
	size_t left = 0;
	size_t right = main.size();
	if (pend.hasBound)
		right = findBoundPosition(main, pend.bound);
	comparisons += worstBinaryComparisons(right);
	while (left < right)
	{
		size_t mid = left + (right - left) / 2;

		if (value.value < main[mid].value)
			right = mid;
		else
			left = mid + 1;
	}
	return left;
}

template <typename Seq, typename PendSeq>
static void insertPend(
	Seq& main,
	PendSeq& pend,
	size_t& comparisons
)
{
	if (pend.empty())
		return;
	std::vector<bool> used(pend.size(), false);
	size_t firstPos = binaryInsertPosition(main, pend[0].value, pend[0], comparisons);
	main.insert(main.begin() + firstPos, pend[0].value);
	used[0] = true;
	std::vector<size_t> order = jacobsthalOrder(pend.size());
	for (size_t i = 0; i < order.size(); i++)
	{
		size_t index = order[i];
		if (index >= pend.size() || used[index])
			continue;
		size_t pos = binaryInsertPosition(main, pend[index].value, pend[index], comparisons);
		main.insert(main.begin() + pos, pend[index].value);
		used[index] = true;
	}
	for (size_t i = pend.size(); i > 0; i--)
	{
		size_t index = i - 1;
		if (used[index])
			continue;
		size_t pos = binaryInsertPosition(main, pend[index].value, pend[index], comparisons);
		main.insert(main.begin() + pos, pend[index].value);
		used[index] = true;
	}
}

template <typename Seq>
static Seq fordJohnsonSort(Seq input, size_t& comparisons)
{
	if (input.size() <= 1)
		return input;
	std::vector<PairNode> pairs;
	typename PendContainer<Seq>::type pend;
	Seq main;
	bool hasExtra = false;
	Element extra;
	for (size_t i = 0; i + 1 < input.size(); i += 2)
	{
		Element first = input[i];
		Element second = input[i + 1];
		comparisons++;
		if (first.value > second.value)
		{
			Element tmp = first;
			first = second;
			second = tmp;
		}
		PairNode pair;
		pair.small = first;
		pair.big = second;
		pairs.push_back(pair);
		main.push_back(pair.big);
	}
	if (input.size()% 2 != 0)
	{
		hasExtra = true;
		extra = input[input.size() - 1];
	}
	Seq sortedMain = fordJohnsonSort(main, comparisons);
	for (size_t i = 0; i <sortedMain.size(); i++)
	{
		for (size_t j =0; j < pairs.size(); j++)
		{
			if (pairs[j].big.id == sortedMain[i].id)
			{
				PendNode node;
				node.value = pairs[j].small;
				node.bound = pairs[j].big;
				node.hasBound = true;
				pend.push_back(node);
				break;
			}
		}
	}
	if (hasExtra)
	{
		PendNode node;
		node.value = extra;
		node.bound = extra;
		node.hasBound = false;
		pend.push_back(node);
	}
	insertPend(sortedMain, pend, comparisons);
	return sortedMain;
}

void PmergeMe::sort()
{
	clock_t start;
	clock_t end;
	std::vector<Element> vectorElements;
	for (size_t i = 0; i < _vector.size(); i++)
	{
		Element e;
		e.value = _vector[i];
		e.id = i;
		vectorElements.push_back(e);
	}
	start = clock();
	std::vector<Element> sortedVector = fordJohnsonSort(vectorElements, _vectorComparisons);
	end = clock();
	_vectorTime = static_cast<double>(end - start) * 1000000.0 / CLOCKS_PER_SEC;
	_vector.clear();
	for (size_t i = 0; i < sortedVector.size(); i++)
		_vector.push_back(sortedVector[i].value);
	std::deque<Element> dequeElements;
	for (size_t i = 0; i < _deque.size(); i++)
	{
		Element e;
		e.value = _deque[i];
		e.id = i;
		dequeElements.push_back(e);
	}
	start = clock();
	std::deque<Element> sortedDeque = fordJohnsonSort(dequeElements, _dequeComparisons);
	end = clock();
	_dequeTime = static_cast<double>(end - start) * 1000000.0 / CLOCKS_PER_SEC;
	_deque.clear();
	for (size_t i = 0; i < sortedDeque.size(); i++)
		_deque.push_back(sortedDeque[i].value);
}

void PmergeMe::displayBefore() const
{
	std::cout << "Before: ";
	for (size_t i = 0; i < _vector.size(); i++)
	{
		std::cout << _vector[i];
		if (i + 1 < _vector.size())
			std::cout << " ";
	}
	std::cout << std::endl;
}

void PmergeMe::displayAfter() const
{
	std::cout << "After: ";
	for (size_t i = 0; i < _vector.size(); i++)
	{
		std::cout << _vector[i];
		if (i + 1 < _vector.size())
			std::cout << " ";
	}
	std::cout << std::endl;
}

void PmergeMe::displayTime() const
{
	std::cout << "Time to process a range of " << _vector.size()
			  << " elements with std::vector : "
			  << _vectorTime << " us" << std::endl;

	std::cout << "Time to process a range of "<< _deque.size()
			<< " elements with std::deque  : "
			<< _dequeTime << " us"<< std::endl;
	std::cout << "Comparisons with std::vector : "
			<< _vectorComparisons << std::endl;
	std::cout << "Comparisons with std::deque  : " 
			<< _dequeComparisons << std::endl;
}
