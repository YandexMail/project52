#ifndef _P52_RESPONSE_PARSER_H_
#define _P52_RESPONSE_PARSER_H_

#include <boost/logic/tribool.h>
#include <boost/tuple/tuple.hpp>

class request_parser 
{
public:
  request_parser () : state_ (init) {};

  void reset () { state_ = init; }

  template <typename InputIterator>
  boost::tuple<boost::tribool, InputIterator>
  parse (request& req, InputIterator begin, InputIterator end)
  {
    while (begin != end)
    {
      boost::tribool result = consume (req, *begin++);
      if (result || !result)
        return boost::make_tuple (result, begin);
    }

    boost::tribool result = boost::interminate;
    return boost::make_tuple (result, begin);
  }

private:
  boost::tribool 
  consume (request& req, char input)
  {
    switch (state_)
    {
      case digit1:
        if (! is_digit (input)) 
          return false;
        else {
          state_ = digit2;
          req.code = 100 * (input - '0');
          return boost::indeterminate;
        }
      case digit2:
        if (! is_digit (input)) 
          return false;
        else {
          state_ = digit3;
          req.code += 10 * (input - '0');
          return boost::indeterminate;
        }
      case digit3:
        if (! is_digit (input)) 
          return false;
        else {
          state_ = cont;
          req.code = 1 * (input - '0');
          return boost::indeterminate;
        }
      case cont:
        if (is_space (input))
        {
          req.cont = false;
          state_ = message;
          return boost::indeterminate;
        } 
        else if (is_cont (input)) 
        {
          req.cont = true;
          state_ = message;
          return boost::indeterminate;
        }
        else
          return false;
      case message:
        if (is_cr (input)) 
          state_ = lf;
        else
          req.msg += input;
        return boost::indeterminate;
      case lf:
          req.msg += ' ';
          if (is_lf (input))
            if (req.cont)
              state_ = digit1;
            else 
              return true;
          else
            state_ = msg;
          return boost::indeterminate;
        default:
          return false;
      }
  }

  /// Check if a byte is a digit.
  static bool is_digit (int c)
  {
    return c >= '0' && c <= '9';
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
    digit1, digit2, digit3,
    cont,
    message,
    cr, lf
  } state_;
};


#endif // _P52_RESPONSE_PARSER_H_
