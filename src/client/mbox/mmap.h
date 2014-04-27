#ifndef _P52_MBOX_MMAP_H_
#define _P52_MBOX_MMAP_H_

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <stdexcept>

namespace p52 {
namespace mbox {

namespace { 
const unsigned int page_size = getpagesize ();
}

class mmap 
{
public:
  typedef char       *       iterator;
  typedef char const * const_iterator;

  mmap () = default;

  mmap (char const* file) 
  {
    open (file);
  }

  void open (char const* file)
  {
    if (fd_ != -1)
      throw std::runtime_error ("mbox file already opened");

    fd_ = ::open (file, O_RDONLY);
    if (fd_ == -1)
      throw std::runtime_error ("cannot open mbox file");

    struct ::stat st;
    if (fstat (fd_, &st) == -1)
      throw std::runtime_error ("cannot fstat mbox file");

    len_ = st.st_size;
    void* ret = ::mmap (0, len_, PROT_READ, MAP_PRIVATE|MAP_FILE, fd_, 0);
    if (ret == MAP_FAILED)
      throw std::runtime_error ("cannot mmap the file");
      
    addr_ = static_cast<char*> (ret);
  }

  void close ()
  {
    if (::munmap (addr_, len_) == -1)
      throw std::runtime_error ("cannot unmap mbox");

    addr_ = 0;
    len_ = 0;

    int rc = ::close (fd_);
    fd_ = -1;
    if (rc == -1)
      throw std::runtime_error ("cannot close mbox");
  }

  ~mmap ()
  {
    if (addr_ != 0) ::munmap (addr_, len_);
    if (fd_ != -1) ::close (fd_);
  }

  iterator begin () { return addr_; }
  iterator   end () { return addr_ + len_; }

  const_iterator begin () const { return cbegin (); }
  const_iterator   end () const { return cend (); }

  const_iterator cbegin () const { return addr_; }
  const_iterator   cend () const { return addr_ + len_; }

  std::size_t size () const { return len_; }

private:
  int fd_ = -1;

  char* addr_;
  std::size_t len_;
};

}}
#endif // _P52_MBOX_MMAP_H_
