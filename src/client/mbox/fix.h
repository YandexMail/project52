#ifndef _P52_MBOX_FIX_H_
#define _P52_MBOX_FIX_H_

#include <stdlib.h> // abort ()

namespace p52 {
namespace mbox {

// Fix message structure.
// 1. adds LF after CR if missing.
// 2. duplicates dot ('.') at the begining of the string.

struct fix_message_parser
{
	enum result_type { r_default, r_cr, r_lf, r_dot };
	enum state_type { s_init, s_default, s_cr, s_lf };

	state_type state_ = s_init;

  fix_message_parser () {}

  void reset () { state_ = s_init; }

  template <typename BufferSequence, typename InputIterator>
  static void insert (BufferSequence& bufs, 
      InputIterator const& b, InputIterator const& e) 
  {
    bufs.emplace_back (&*b, e-b);
  }

  template <typename BufferSequence>
  static void insert_cr (BufferSequence& bufs)
  {
  	static const char cr = '\r';
  	bufs.emplace_back (&cr, 1);
  }

  template <typename BufferSequence>
  static void insert_lf (BufferSequence& bufs)
  {
  	static const char lf = '\n';
  	bufs.emplace_back (&lf, 1);
  }

  template <typename BufferSequence>
  static void insert_dot (BufferSequence& bufs)
  {
  	static const char dot = '.';
  	bufs.emplace_back (&dot, 1);
  }

  template <typename BufferSequence, typename InputIterator>
  void parse (BufferSequence& bufs, InputIterator begin, InputIterator const& end)
  {
  	InputIterator current = begin;

  	for (; current != end; ++current)
    {
    	auto result = consume (*current);

    	if (result != r_default && current != begin)
      {
    		insert (bufs, begin, current);
    		begin = current;
      }
      	
    	if (result == r_cr) insert_cr (bufs);
    	if (result == r_lf) insert_lf (bufs);
    	if (result == r_dot) insert_dot (bufs);
    }

    if (begin != end)
      insert (bufs, begin, end);
   }

  template <typename BufferSequence>
  void parse_end (BufferSequence& bufs)
  {
  	if (state_ != s_init)
    {
    	if (state_ != s_cr) insert_cr (bufs);
    	insert_lf (bufs);
    }

    // cheating: insert smtp end of data marker.
    insert_dot (bufs);
    insert_cr (bufs);
    insert_lf (bufs);

    reset ();
  }

  void print (char input) 
  {
#if 0
  	if (state_ == s_default) std::cout << "s_default ";
  	if (state_ == s_init) std::cout << "s_init    ";
  	if (state_ == s_cr) std::cout << "s_cr      ";

  	if (is_cr (input)) std::cout << "CR";
  	else if (is_lf (input)) std::cout << "LF";
  	else std::cout << input;

  	std::cout << "\n";
#else
		(void) input;
#endif
  }

  result_type consume (char input)
  {
      print (input);
  	switch (state_)
    {
    	case s_init: 
	state_ = s_default;
       	if (is_dot (input))
	{ 
		return r_dot;
	}

      case s_default:
        if (is_lf (input))
        {
        	state_ = s_init;
        	return r_cr;
        }

        if (is_cr (input))
        {
        	state_ = s_cr;
        	return r_default;
        }

        return r_default;

      case s_cr: 
        state_ = is_lf (input) ? s_init : s_default;
        return r_default;

      default:
        abort ();
    }
  }

  static bool is_dot (int c) { return c == '.'; }
  static bool is_cr (int c) { return c == '\r'; }
  static bool is_lf (int c) { return c == '\n'; }

};

}}
#endif // _P52_MBOX_FIX_H_
