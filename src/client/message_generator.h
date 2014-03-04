#ifndef _P52_MESSAGE_GENERATOR_H_
#define _P52_MESSAGE_GENERATOR_H_

struct message_generator 
{
  template <typename Handler>
  void operator() (Handler h) const
  {
    h ("", "", "hello world");
  }
};

#endif // _P52_MESSAGE_GENERATOR_H_
