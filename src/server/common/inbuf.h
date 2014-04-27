#ifndef _P52_INBUF_H_
#define _P52_INBUF_H_
#include <streambuf>
#include <boost/asio.hpp>
#include <memory>

template <typename ReadFunc>
class inbuf : public std::streambuf
{
private:
    static const std::streamsize        pb_size;

    enum
    { bf_size = 16 };

    int fetch_()
    {
        std::streamsize num = std::min(
            static_cast< std::streamsize >( gptr() - eback() ), 
            pb_size);

        std::memmove(
            buffer_ + ( pb_size - num),
            gptr() - num, 
            num);

        boost::system::error_code ec;
        std::size_t n = read_func_ (ec, 
            boost::asio::buffer( buffer_ + pb_size, bf_size - pb_size));

        // check if an error occurred
        if (ec)
        {
            setg( 0, 0, 0);
            return -1;
        }

        setg(
            buffer_ + pb_size - num, 
            buffer_ + pb_size, 
            buffer_ + pb_size + n);
        
        return n;
    }

    ReadFunc                              read_func_;
    char                                  buffer_[bf_size];

protected:
    virtual int underflow()
    {
        if ( gptr() < egptr() )
            return traits_type::to_int_type( * gptr() );

        if ( 0 > fetch_() )
            return traits_type::eof();
        else
            return traits_type::to_int_type( * gptr() );
    }

public:
    inbuf(ReadFunc read_func) 
      : read_func_ (std::move (read_func)), buffer_()
    { 
      setg( buffer_ + 4, buffer_ + 4, buffer_ + 4); 
    }

    inbuf (inbuf const&) = delete;
    inbuf& operator= (inbuf const&) = delete;

    inbuf (inbuf&&) = default;
    inbuf& operator= (inbuf&&) = default;
};

template <class ReadFunc>
const std::streamsize inbuf<ReadFunc>::pb_size = 4;

template <typename ReadFunc>
std::unique_ptr<inbuf<ReadFunc>>
make_inbuf_ptr (ReadFunc&& rf)
{
  return std::unique_ptr<inbuf<ReadFunc>> (
      new inbuf<ReadFunc> (std::forward<ReadFunc> (rf)));
}
#endif // _P52_INBUF_H_
