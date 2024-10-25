#ifndef IMAP_H
#define IMAP_H
#include "openssl/bio.h"
#include <iostream>
#include <filesystem>
#include <vector>
#include <sstream>
using namespace std;

bool login(BIO *bio, string username, string password);
void logout(BIO *bio);
int fetchMail(BIO *bio, string mailbox, string outDir, bool onlyNew, bool headersOnly);
#endif // IMAP_H