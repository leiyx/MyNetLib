/*
 * @Author: lei
 * @Description: socket工具类
 * @FilePath: /MyNetLib/include/socket_utils.h
 */
#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "logger.h"
namespace socket_utils {

void SetReuseAddr(int fd, bool on);
void SetReusePort(int fd, bool on);
void SetKeepAlive(int fd, bool on);
const sockaddr_in GetLocalAddr(int sockfd);
const sockaddr_in GetPeerAddr(int sockfd);
bool SetNonblocking(int fd, bool on);

}  // namespace socket_utils
#endif