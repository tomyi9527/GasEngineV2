#pragma once

namespace ParsingUtility
{
	const double fast_atof_table[16] = {  // we write [16] here instead of [] to work around a swig bug
		0.0,
		0.1,
		0.01,
		0.001,
		0.0001,
		0.00001,
		0.000001,
		0.0000001,
		0.00000001,
		0.000000001,
		0.0000000001,
		0.00000000001,
		0.000000000001,
		0.0000000000001,
		0.00000000000001,
		0.000000000000001
	};

#define AI_FAST_ATOF_RELAVANT_DECIMALS 15
#define MAXLEN 1024

	inline bool IsLineEnd(char in)
	{
		return (in == (char)'\r' || in == (char)'\n' || in == (char)'\0' || in == (char)'\f');
	}

	inline bool IsSpace(char in)
	{
		return (in == (char)' ' || in == (char)'\t');
	}

	inline bool SkipSpaces(const char* in, const char** out)
	{
		while (*in == (char)' ' || *in == (char)'\t')
		{
			++in;
		}

		*out = in;
		return !IsLineEnd(*in);
	}

	inline bool SkipSpaces(const char** inout)
	{
		return SkipSpaces(*inout, inout);
	}

	inline bool SkipLine(const char* in, const char** out)
	{
		while (*in != (char)'\r' && *in != (char)'\n' && *in != (char)'\0')
		{
			++in;
		}

		// files are opened in binary mode. Ergo there are both NL and CR
		while (*in == (char)'\r' || *in == (char)'\n')
		{
			++in;
		}
		*out = in;
		return *in != (char)'\0';
	}

	inline bool IsSpaceOrNewLine(char in)
	{
		return IsSpace(in) || IsLineEnd(in);
	}

	inline bool SkipSpacesAndLineEnd(const char* in, const char** out)
	{
		while (*in == (char)' ' || *in == (char)'\t' || *in == (char)'\r' || *in == (char)'\n') {
			++in;
		}
		*out = in;
		return *in != '\0';
	}

	inline bool SkipSpacesAndLineEnd(const char** inout)
	{
		return SkipSpacesAndLineEnd(*inout, inout);
	}

	inline int Strincmp(const char *s1, const char *s2, unsigned int n)
	{
		if (!n)
		{
			return 0;
		}

#ifdef _MSC_VER
		return ::_strnicmp(s1, s2, n);
#else
		return ::strncasecmp(s1, s2, n);
		//char c1, c2;
		//unsigned int p = 0;
		//do
		//{
		//	if (p++ >= n)return 0;
		//	c1 = tolower(*s1++);
		//	c2 = tolower(*s2++);
		//} while (c1 && (c1 == c2));

		//return c1 - c2;
#endif
	}

	inline unsigned long long strtoul10_64(const char* in, const char** out = 0, unsigned int* max_inout = 0)
	{
		unsigned int cur = 0;
		unsigned long long value = 0;

		if (*in < '0' || *in > '9')
			throw std::string("The string cannot be converted into a value.");

		bool running = true;
		while (running)
		{
			if (*in < '0' || *in > '9')
				break;

			const unsigned long long new_value = (value * 10) + (*in - '0');

			// numeric overflow, we rely on you
			if (new_value < value) {
				throw std::string("Converting the string into a value resulted in overflow.");
				return 0;
			}
			//throw std::overflow_error();

			value = new_value;

			++in;
			++cur;

			if (max_inout && *max_inout == cur)
			{
				if (out)
				{ /* skip to end */
					while (*in >= '0' && *in <= '9')
					{
						++in;
					}
					*out = in;
				}

				return value;
			}
		}
		if (out)
			*out = in;

		if (max_inout)
			*max_inout = cur;

		return value;
	}

	inline const char* fast_atoreal_move(const char* c, double& out, bool check_comma = true)
	{
		double f = 0;

		bool inv = (*c == '-');
		if (inv || *c == '+')
		{
			++c;
		}

		if ((c[0] == 'N' || c[0] == 'n') && Strincmp(c, "nan", 3) == 0)
		{
			out = NAN;
			c += 3;
			return c;
		}

		if ((c[0] == 'I' || c[0] == 'i') && Strincmp(c, "inf", 3) == 0)
		{
			out = INFINITY;
			if (inv)
			{
				out = -out;
			}
			c += 3;
			if ((c[0] == 'I' || c[0] == 'i') && Strincmp(c, "inity", 5) == 0)
			{
				c += 5;
			}
			return c;
		}

		if (!(c[0] >= '0' && c[0] <= '9') &&
			!((c[0] == '.' || (check_comma && c[0] == ',')) && c[1] >= '0' && c[1] <= '9'))
		{
			throw std::string("Cannot parse string "
				"as real number: does not start with digit "
				"or decimal point followed by digit.");
		}

		if (*c != '.' && (!check_comma || c[0] != ','))
		{
			f = static_cast<double>(strtoul10_64(c, &c));
		}

		if ((*c == '.' || (check_comma && c[0] == ',')) && c[1] >= '0' && c[1] <= '9')
		{
			++c;

			// NOTE: The original implementation is highly inaccurate here. The precision of a single
			// IEEE 754 float is not high enough, everything behind the 6th digit tends to be more
			// inaccurate than it would need to be. Casting to double seems to solve the problem.
			// strtol_64 is used to prevent integer overflow.

			// Another fix: this tends to become 0 for long numbers if we don't limit the maximum
			// number of digits to be read. AI_FAST_ATOF_RELAVANT_DECIMALS can be a value between
			// 1 and 15.
			unsigned int diff = AI_FAST_ATOF_RELAVANT_DECIMALS;
			double pl = static_cast<double>(strtoul10_64(c, &c, &diff));

			pl *= fast_atof_table[diff];
			f += static_cast<double>(pl);
		}
		// For backwards compatibility: eat trailing dots, but not trailing commas.
		else if (*c == '.') 
		{
			++c;
		}

		// A major 'E' must be allowed. Necessary for proper reading of some DXF files.
		// Thanks to Zhao Lei to point out that this if() must be outside the if (*c == '.' ..)
		if (*c == 'e' || *c == 'E')
		{
			++c;
			const bool einv = (*c == '-');
			if (einv || *c == '+')
			{
				++c;
			}

			// The reason float constants are used here is that we've seen cases where compilers
			// would perform such casts on compile-time constants at runtime, which would be
			// bad considering how frequently fast_atoreal_move<float> is called in Assimp.
			double exp = static_cast<double>(strtoul10_64(c, &c));
			if (einv)
			{
				exp = -exp;
			}
			f *= pow(static_cast<double>(10.0), exp);
		}

		if (inv)
		{
			f = -f;
		}
		out = f;
		return c;
	}
};