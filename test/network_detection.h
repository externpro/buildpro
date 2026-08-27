#ifndef NETWORK_DETECTION_H
#define NETWORK_DETECTION_H

#ifdef _WIN32
#include <windows.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#endif

// Ultra-fast network detection - just try a quick connection with 500ms timeout
inline bool isNetworkAvailable()
{
#ifdef _WIN32
  // Initialize Winsock
  WSADATA wsaData;
  int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (wsaResult != 0)
  {
    return false;
  }

  SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock == INVALID_SOCKET)
  {
    WSACleanup();
    return false;
  }

  // Set socket to non-blocking mode
  u_long mode = 1;
  ioctlsocket(sock, FIONBIO, &mode);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(53);
  addr.sin_addr.s_addr = inet_addr("8.8.8.8");

  int result = connect(sock, (struct sockaddr*)&addr, sizeof(addr));

  bool networkAvailable = false;
  if (result == SOCKET_ERROR)
  {
    int error = WSAGetLastError();
    if (error == WSAEWOULDBLOCK)
    {
      fd_set writefds;
      FD_ZERO(&writefds);
      FD_SET(sock, &writefds);

      struct timeval timeout;
      timeout.tv_sec = 0;
      timeout.tv_usec = 500000; // 500ms

      // On Windows, first parameter to select is ignored but should be 0
      int selectResult = select(0, nullptr, &writefds, nullptr, &timeout);

      if (selectResult > 0)
      {
        // Check if connection succeeded
        int error = 0;
        int len = sizeof(error);
        getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
        networkAvailable = (error == 0);
      }
    }
  }
  else
  {
    networkAvailable = true;
  }

  closesocket(sock);
  WSACleanup();
  return networkAvailable;
#else
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
  {
    return false;
  }

  // Set socket to non-blocking mode
  int flags = fcntl(sock, F_GETFL, 0);
  fcntl(sock, F_SETFL, flags | O_NONBLOCK);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(53);
  addr.sin_addr.s_addr = inet_addr("8.8.8.8");

  int result = connect(sock, (struct sockaddr*)&addr, sizeof(addr));

  if (result < 0)
  {
    if (errno == EINPROGRESS)
    {
      fd_set writefds;
      FD_ZERO(&writefds);
      FD_SET(sock, &writefds);

      struct timeval timeout;
      timeout.tv_sec = 0;
      timeout.tv_usec = 500000; // 500ms

      int selectResult =
        select(sock + 1, nullptr, &writefds, nullptr, &timeout);

      if (selectResult > 0)
      {
        // Check if connection succeeded
        int error = 0;
        socklen_t len = sizeof(error);
        getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len);
        close(sock);
        return (error == 0);
      }

      close(sock);
      return false;
    }
  }

  close(sock);
  return true;
#endif
}

#endif // NETWORK_DETECTION_H
