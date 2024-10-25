/**
 * @file connect.h
 * @brief Connection utility functions for the IMAPCL project.
 * @project IMAPCL
 * @date 2024-10-24
 * @author Jan Osuský
 * @login xosusk00
 */
#ifndef CONNECT_H
#define CONNECT_H

#include <iostream>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "openssl/bio.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

using namespace std;

void InitializeOpenSSL();
BIO *connectToServer(string server, string port, bool useTLS, string certFile, string certDir, SSL_CTX **ctx);

#endif // CONNECT_H