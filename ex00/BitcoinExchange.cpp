#include "BitcoinExchange.hpp"

BitcoinExchange::BitcoinExchange(){}
BitcoinExchange::~BitcoinExchange(){}
BitcoinExchange::BitcoinExchange(const BitcoinExchange& other)
{
	*this = other;
}

BitcoinExchange &BitcoinExchange::operator=(const BitcoinExchange& other)
{
	if (this != &other)
		this->_database = other._database;
	return *this;
}

void BitcoinExchange::loadDatabase(const std::string& filename)
{
	std::ifstream file(filename.c_str());
	if (!file.is_open())
		throw std::runtime_error("Error: could not open database file");
	std::string line;
	std::getline(file, line);
	while (std::getline(file, line))
	{
		std::stringstream ss(line); // hayde for extract men line for example to extract 2001-01-04 so it turns the line in to something we can split
		std::string date;
		std::string priceStr;
		if (!std::getline(ss, date, ','))//gets everything before comma
			continue;
		if (!std::getline(ss, priceStr))//gets  everything after comma
			continue;
		double price = std::atof(priceStr.c_str());
		_database[date] = price;
	}
	file.close();
	if (_database.empty())
		throw std::runtime_error("Error: empty database");
}

double BitcoinExchange::getRate(const std::string& date) const
{
	std::map<std::string, double>::const_iterator it;
	it = _database.find(date);
	if (it != _database.end())
		return it->second;
	it = _database.upper_bound(date);
	if (it == _database.begin())
		throw std::runtime_error("Error: no earlier date in database");
	--it;
	return it->second;
}


static bool isValidNumber(const std::string& value)
{
	bool hasDigit = false;
	bool hasDot = false;
	if (value.empty())
		return false;
	size_t start = 0;
	if (value[0] == '-')
		start = 1;
	if (start >= value.length())
		return false;
	for (size_t i = start; i < value.length(); i++)
	{
		if (std::isdigit(value[i]))
			hasDigit = true;
		else if (value[i] == '.')
		{
			if (hasDot)
				return false;
			hasDot = true;
		}
		else
			return false;
	}
	return hasDigit;
}

static bool isLeapYear(int year)
{
	if (year % 400 == 0)
		return true;
	if (year % 100 == 0)
		return false;
	if (year % 4 == 0)
		return true;
	return false;
}

bool BitcoinExchange::parseAndNormalizeDate(const std::string& dateStr, std::string& normalized, int& year, int& month, int& day) const
{
	size_t firstDash = dateStr.find('-');
	if (firstDash == std::string::npos)
		return false;
	size_t secondDash = dateStr.find('-', firstDash + 1);
	if (secondDash == std::string::npos)
		return false;
	if (dateStr.find('-', secondDash + 1) != std::string::npos)
		return false;
	std::string yearStr = dateStr.substr(0, firstDash);
	std::string monthStr = dateStr.substr(firstDash + 1, secondDash - (firstDash + 1));
	std::string dayStr = dateStr.substr(secondDash + 1);
	if (yearStr.size() != 4)
		return false;
	if (monthStr.empty() || monthStr.size() > 2)
		return false;
	if (dayStr.empty() || dayStr.size() > 2)
		return false;
	for (size_t i = 0; i < yearStr.size(); i++)
		if (!std::isdigit(static_cast<unsigned char>(yearStr[i])))
			return false;
	for (size_t i = 0; i < monthStr.size(); i++)
		if (!std::isdigit(static_cast<unsigned char>(monthStr[i])))
			return false;
	for (size_t i = 0; i < dayStr.size(); i++)
		if (!std::isdigit(static_cast<unsigned char>(dayStr[i])))
			return false;
	year = std::atoi(yearStr.c_str());
	month = std::atoi(monthStr.c_str());
	day = std::atoi(dayStr.c_str());
	if (year < 0)
		return false;
	if (month < 1 || month > 12)
		return false;
	int daysInMonth[] = {
		31, 28, 31, 30, 31, 30,
		31, 31, 30, 31, 30, 31
	};
	if (month == 2 && isLeapYear(year))
		daysInMonth[1] = 29;
	if (day < 1 || day > daysInMonth[month - 1])
		return false;
	std::ostringstream out;
	out << std::setw(4) << std::setfill('0') << year << "-";
	out << std::setw(2) << std::setfill('0') << month << "-";
	out << std::setw(2) << std::setfill('0') << day;
	normalized = out.str();
	return true;
}

bool BitcoinExchange::isFutureDate(int year, int month, int day) const
{
	std::time_t now = std::time(0);
	std::tm* local = std::localtime(&now);
	if (!local)
		return false;
	int currentYear = local->tm_year + 1900;
	int currentMonth = local->tm_mon + 1;
	int currentDay = local->tm_mday;
	if (year > currentYear)
		return true;
	if (year < currentYear)
		return false;
	if (month >currentMonth)
		return true;
	if (month < currentMonth)
		return false;
	return day > currentDay;
}

bool BitcoinExchange::isValidDate(const std::string& date) const
{
	std::string normalized;
	int year;
	int month;
	int day;
	return parseAndNormalizeDate(date, normalized, year, month, day);
}

bool BitcoinExchange::isValidValue(const std::string& valueStr, double& value) const
{
	if (!isValidNumber(valueStr))
		return false;
	value = std::atof(valueStr.c_str());
	if (value < 0 || value > 1000)
		return false;
	return true;
}

static std::string trim(const std::string& str)
{
	size_t start = 0;
	size_t end = str.length();
	while (start < end && (str[start] == ' ' || str[start] == '\t'))
		start++;
	while (end > start && (str[end - 1] == ' ' || str[end - 1] == '\t' || str[end - 1] == '\r' || str[end - 1] == '\n'))
		end--;
	return str.substr(start, end - start);
}

void BitcoinExchange::processInput(const std::string& filename) const
{
	std::ifstream file(filename.c_str());
	if (!file.is_open())
		throw std::runtime_error("Error: could not open file");
	std::string line;
	std::getline(file, line); // this to skip line of       date | value <-- skipped
	while (std::getline(file, line))
	{
		std::stringstream ss(line);// convert string in to stream so we can split it
		std::string date;
		std::string valueStr;
		if (!std::getline(ss, date, '|') || !std::getline(ss, valueStr))
		{
			std::cout << "Error: bad input => " << line << std::endl;
			continue;
		}
		date = trim(date);
		valueStr = trim(valueStr);
		std::string normalizedDate;
		int year;
		int month;
		int day;
		if (!parseAndNormalizeDate(date, normalizedDate, year, month, day))
		{
			std::cout << "Error: bad input => " << line << std::endl;
			continue;
		}
		if (isFutureDate(year, month, day))
		{
			std::cout << "Error: date is in the future." << std::endl;
			continue;
		}
		if (!isValidNumber(valueStr))
		{
			std::cout << "Error: bad input => " << line << std::endl;
			continue;
		}
		double value = std::atof(valueStr.c_str());
		if (value < 0)
		{
			std::cout << "Error: not a positive number." << std::endl;
			continue;
		}
		if (value > 1000)
		{
			std::cout << "Error: too large a number." << std::endl;
			continue;
		}
		try
		{
			double rate = getRate(normalizedDate);
			std::cout << normalizedDate << " => " << valueStr
				<< " = " << value * rate << std::endl;
		}
		catch (const std::exception& e)
		{
			std::cout << e.what()<< std::endl;
		}
	}
	file.close();
}

