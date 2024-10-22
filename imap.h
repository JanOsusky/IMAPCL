#ifndef IMAP_H
#define IMAP_H
#include "openssl/bio.h"
#include <iostream>
using namespace std;

bool login(BIO *bio, string username, string password);
void logout(BIO *bio);

#endif // IMAP_H