/*
This file is part of MAE.

History:

[2006.02.28 - Helio Perroni Filho] Created.
*/

#ifndef __MAE_REGEX_PATTERN
#define __MAE_REGEX_PATTERN

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace mae
{
	namespace regex
	{
		class Pattern;
	};
};

/**
A regular expression pattern. Can be used to query whether a given string matches a pattern, as well as retrieving selected substrings from it.
*/
class mae::regex::Pattern
{
	/*
	Attribute Section
	*/
	
	/** The matching pattern string. */
	private: string pattern;
	
	/** POSIX structure representing the matching pattern. */
	private: regex_t compiledPattern;
	
	/*
	Constructor Section
	*/
	
	/**
	Creates a new matching pattern out of a pattern string.
	
	@param pattern Matching pattern string.
	*/
	public: Pattern(string pattern);
	
	/*
	Method Section
	*/
	
	/**
	Returns whether this pattern matches a given input string.
	
	@param input The input string.
	
	@return Whether this pattern matches a given input string.
	*/
	public: bool matches(string input);
	
	/**
	Returns whether this pattern matches a given input string. If the input string contains subexpressions, the matched input substrings are returned through a vector.
	
	@param input The input string.
	
	@param substrings Vector used to return the matched substrings.
	
	@return Whether this pattern matches a given input string.
	*/
	public: bool matches(string input, vector<string>& substrings);

	/**
	Returns whether a pattern matches a given input string.

	@param pattern Matching pattern string.
	
	@param input The input string.
	
	@return Whether this pattern matches a given input string.
	*/
	public: static bool matches(string pattern, string input);
	
	/**
	Returns whether a pattern matches a given input string. If the input string contains subexpressions, the matched input substrings are returned through a vector.

	@param pattern Matching pattern string.
	
	@param input The input string.
	
	@param substrings Vector used to return the matched substrings.
	
	@return Whether this pattern matches a given input string.
	*/
	public: static bool matches(string pattern, string input, vector<string>& substrings);
	
	/*
	Property Section
	*/
	
	/**
	Returns the string representation for this pattern.
	
	@return The string representation for this pattern.
	*/
	public: string getPattern();
	
	/**
	Sets the string representation for this pattern.
	
	@param pattern The string representation for this pattern.
	*/
	public: void setPattern(string pattern);
};

#endif
