#include "buffer.h"

#include <errno.h>
#include <memory.h>
#include <sys/uio.h>

namespace easynet {
ssize_t Buffer::readFd(int fd, int* savedErrno) {
  // stack临时缓冲区，64KB，用于一次read接受全部数据
  char extrabuf[65536];
  struct iovec vec[2];

  const size_t writable = writableBytes();
  vec[0].iov_base = begin() + m_writeIndex;
  vec[0].iov_len = writable;
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof(extrabuf);

  const ssize_t n = readv(fd, vec, 2);
  if (n < 0) {
    *savedErrno = errno;
  } else if (static_cast<size_t>(n) <= writable) {
    m_writeIndex += n;
  } else {
    m_writeIndex = m_buffer.size();
    append(extrabuf, n - writable);
  }
  return n;
}
}  // namespace easynet