#include <iostream>
#include <iterator>
#include "rfc822_v2.h"

std::string str = {
#if 0
"v:h\r\na:b\r\n\r\nlvfmklv\r\n"
#else
"Received: from mxcorp1.mail.yandex.net ([127.0.0.1])\r\n\
  by: mxcorp1.mail.yandex.net with LMTP id 6IISZj4b\r\n\
    for: <help@mail.yandex-team.ru>; Wed, 5 Sep 2012 13:06:18 +0400\r\n\
Received: from gamgee.yandex.ru (gamgee.yandex.ru [77.88.19.54])\r\n\
         by: mxcorp1.mail.yandex.net (nwsmtp/Yandex) with ESMTP id 6IYSrTpv-6IY8wdri;\r\n\
  Wed:,  5 Sep 2012 13:06:18 +0400\r\n\
\r\n\
rklmrk\r\n\
eflvfrjklv\r\n\
ejklvfn\r\n\
"
#endif
};

int main ()
{
	auto iter = str.begin ();
  bool r = rfc822::parse (iter, str.end ());
  if (r) 
  {
  	std::cout << "success\n";
  }
  else
  {
  	std::cout << "fail\n";
  }
}
