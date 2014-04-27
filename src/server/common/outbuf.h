#ifndef _P52_OUTBUF_H_
#define _P52_OUTBUF_H_
#include <streambuf>
#include <boost/asio.hpp>
#include <memory>

template <typename WriteFunc>
class outbuf : public std::streambuf
{
private:
    enum
    { bf_size = 16 };

    std::streamsize
    do_write_ (char_type const* text, std::streamsize n)
    {
      // std::cout << "   do_write_ (" << n << ");\n";
      boost::system::error_code ec;
      std::size_t out = write_func_ (ec, boost::asio::buffer (text, n));
      return ec ? traits_type::eof () : out;
    }

    inline std::streamsize ready_ () const
    {
      return pptr () - pbase ();
    }

    WriteFunc                             write_func_;
    char_type                             buffer_[bf_size+1];

protected:
    virtual outbuf::int_type overflow (outbuf::int_type c)
    {
      // std::cout << "overflow (" << c << ")=...\n";
      auto rc = overflow_ (c);
      // std::cout << "overflow (" << c << ")=" << rc << "\n";
      return rc;
    }

    outbuf::int_type overflow_ (outbuf::int_type c)
    {
      std::streamsize n = ready_ ();

      if (c != traits_type::eof ())
      {
        *pptr () = c;
        ++n;
      }

      if (n && do_write_ (pbase (), n) != n)
        return traits_type::eof ();

      if (c != traits_type::eof ())
        --n;

      pbump (-n);
      return c;
    }

    virtual int sync ()
    {
      // std::cout << "sync ()=...\n";
      auto rc = sync_ ();
      // std::cout << "sync ()=" << rc <<"\n";
      return rc;
    }

    int sync_ ()
    {
      std::streamsize n = ready_ ();
      // std::cout << "   sync_ (" << n << ");\n";
      if (n)
      {
        std::streamsize out = do_write_ (pbase (), n);
        if (out > 0 && out < n)
          std::memcpy (pbase (), pbase ()+out, epptr ()-pbase ()-out);

        pbump (-out);

        return (out == n) ? 0 : -1;
      }

      return 0;
    }

    virtual std::streamsize 
    xsputn (char_type const* text, std::streamsize n)
    {
      // std::cout << "xsputn (" << n << ")=...\n";
      auto rc = xsputn_ (text, n);
      // std::cout << "xsputn (" << n << ")=" << rc <<"\n";
      return rc;
    }

    bool copy_if_possible (char_type const* s, std::streamsize n)
    {
      if (epptr () - pptr () < n) return false;

      std::memcpy (pptr (), s, n*sizeof (char_type));
      pbump (n);
      return true;
    }

    std::streamsize 
    xsputn_ (char_type const* text, std::streamsize n)
    {
      if (copy_if_possible (text, n))
        return n;

      if (sync () == traits_type::eof ()) 
        return 0;

      if (copy_if_possible (text, n))
        return n;

      return do_write_ (text, n); 
    }

public:
    outbuf(WriteFunc write_func) 
      : write_func_ (write_func), buffer_()
    { 
      setp (buffer_, buffer_ + bf_size); 
    }

    ~outbuf () { sync (); }

    outbuf (outbuf const&) = delete;
    outbuf& operator= (outbuf const&) = delete;

    outbuf (outbuf&&) = default;
    outbuf& operator= (outbuf&&) = default;
};

template <typename WriteFunc>
std::unique_ptr<outbuf<WriteFunc>>
make_outbuf_ptr (WriteFunc&& wf)
{
  return std::unique_ptr<outbuf<WriteFunc>> (
      new outbuf<WriteFunc> (std::forward<WriteFunc> (wf)));
}
#endif // _P52_OUTBUF_H_
