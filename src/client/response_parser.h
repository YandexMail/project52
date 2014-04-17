#ifndef _P52_RESPONSE_PARSER_H_
#define _P52_RESPONSE_PARSER_H_

#include <boost/logic/tribool.hpp>
#include <tuple>

#include "server_response.h"

class response_parser 
{
public:
  response_parser () : state_ (init) {};

  void reset () { state_ = init; }

  typedef boost::tribool result_type;

  template <typename InputIterator>
  std::tuple<result_type, InputIterator>
  parse (server_response& resp, InputIterator begin, InputIterator end)
  {
    while (begin != end)
    {
      auto result = consume (resp, *begin++);
      if (result || !result)
        return std::make_tuple (result, begin);
    }

    result_type result = boost::indeterminate;
    return std::make_tuple (result, begin);
  }

private:

  result_type consume (server_response& resp, char input)
  {
  	// std::cout << "parser: state=" << state_ << ", char=" << input << "\n";
    switch (state_)
    {
    	case init:
    	  resp.msg = "";
    	  resp.cont = false;
    	  resp.code = 0;

      case digit1:
        if (! is_digit (input)) 
          return false;
        else {
          state_ = digit2;
          resp.code = 100 * (input - '0');
          return boost::indeterminate;
        }
      case digit2:
        if (! is_digit (input)) 
          return false;
        else {
          state_ = digit3;
          resp.code += 10 * (input - '0');
          return boost::indeterminate;
        }
      case digit3:
        if (! is_digit (input)) 
          return false;
        else {
          state_ = cont;
          resp.code += 1 * (input - '0');
          return boost::indeterminate;
        }
      case cont:
        if (is_space (input))
        {
          resp.cont = false;
          state_ = message;
          return boost::indeterminate;
        } 
        else if (is_cont (input)) 
        {
          resp.cont = true;
          state_ = message;
          return boost::indeterminate;
        }
        else
          return false;

      case message:
        if (is_cr (input)) 
        {
          state_ = cr;
          return boost::indeterminate;
        }

        if (! is_lf (input)) 
        {
          resp.msg += input;
          return boost::indeterminate;
        }

      case cr:
        if (! is_lf (input))
        	return false;
        
        if (resp.cont) 
        {
        	resp.msg += ' ';
        	state_ = digit1;
          return boost::indeterminate;
        }
        
        return true;

      default:
          return false;
    }
  }

  /// Check if a byte is a digit.
  static bool is_digit (int c)
  {
    return c >= '0' && c <= '9';
  }

  static bool is_space (int c)
  {
  	return c == ' ';
  }

  static bool is_cr (int c)
  {
    return c == '\r';
  }

  static bool is_lf (int c)
  {
    return c == '\n';
  }

  static bool is_cont (int c)
  {
    return c == '-';
  }

  /// The current state of parser.
  enum state {
  	init,
    digit1, digit2, digit3,
    cont,
    message,
    cr
  } state_;
};


#endif // _P52_RESPONSE_PARSER_H_
