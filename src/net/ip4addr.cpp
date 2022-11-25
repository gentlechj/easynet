#include "ip4addr.h"

#include <strings.h>

#include "logging.h"
#include "netutil.h"
#include "util.h"

//     struct sockaddr_in {
//         sa_family_t    sin_family; /* address family: AF_INET */
//         uint16_t       sin_port;   /* port in network byte order */
//         struct in_addr sin_addr;   /* internet address */
//     };

//     typedef uint32_t in_addr_t;
//     struct in_addr {
//         in_addr_t       s_addr;     /* address in network byte order */
//     };

namespace easynet {
static const in_addr_t kInaddrAny = INADDR_ANY;

Ip4Addr::Ip4Addr(const std::string &host, unsigned short port) {
  bzero(&m_address, sizeof(m_address));
  m_address.sin_family = AF_INET;
  m_address.sin_port = htons(port);
  if (host.size()) {
    m_address.sin_addr = net::getHostByName(host);
  } else {
    m_address.sin_addr.s_addr = INADDR_ANY;
  }
  if (m_address.sin_addr.s_addr == INADDR_NONE) {
    error("cannot resove %s to ip", host.c_str());
  }
}
std::string Ip4Addr::toHostPort() const {
  uint32_t uip = m_address.sin_addr.s_addr;
  return format("%d.%d.%d.%d:%d", (uip >> 0) & 0xff, (uip >> 8) & 0xff,
                (uip >> 16) & 0xff, (uip >> 24) & 0xff,
                ntohs(m_address.sin_port));
}

std::string Ip4Addr::ip() const {
  uint32_t uip = m_address.sin_addr.s_addr;
  return format("%d.%d.%d.%d", (uip >> 0) & 0xff, (uip >> 8) & 0xff,
                (uip >> 16) & 0xff, (uip >> 24) & 0xff);
}

unsigned short Ip4Addr::port() const {
  return static_cast<unsigned short>(m_address.sin_port);
}

unsigned int Ip4Addr::ipInt() const { return ntohl(m_address.sin_addr.s_addr); }

bool Ip4Addr::isIpValid() const {
  return m_address.sin_addr.s_addr != INADDR_NONE;
}

}  // namespace easynet