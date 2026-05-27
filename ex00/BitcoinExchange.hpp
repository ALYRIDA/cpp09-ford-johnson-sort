#ifndef BITCOINEXCHANGE_HPP
#define BITCOINEXCHANGE_HPP


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <cstdlib>
#include <stdexcept>
#include <ctime>
#include <cctype>
#include <iomanip>

class BitcoinExchange
{
	private:
		std::map<std::string, double> _database;
	public:
		BitcoinExchange();
		~BitcoinExchange();
		BitcoinExchange(const BitcoinExchange& other);
		BitcoinExchange &operator=(const BitcoinExchange& other);

		void loadDatabase(const std::string& filename);
		void processInput(const std::string& filename) const;
	private:
		bool isValidDate(const std::string& date) const;
		bool isValidValue(const std::string& valueStr, double& value) const;
		double getRate(const std::string& date) const;
		bool parseAndNormalizeDate(const std::string& dateStr, std::string& normalized, int& year, int& month, int& day) const;
		bool isFutureDate(int year, int month, int day) const;
};

#endif
