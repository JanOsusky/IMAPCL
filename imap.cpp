#include "imap.h"

bool login(BIO *bio, string username, string password)
{
    string command = "A001 LOGIN " + username + " " + password + "\r\n";
    if (BIO_write(bio, command.c_str(), command.length()) <= 0)
    {
        if (!BIO_should_retry(bio))
        {
            cerr << "Chyba při odesílání příkazu LOGIN" << endl;
            return false;
        }
    }
    else
    {
        char buffer[4096];
        int len = BIO_read(bio, buffer, 4096); // hardcoded buffer size
        if (len > 0)
        {
            buffer[len] = '\0';
            cerr << "Server response: " << buffer << endl;
            return true;
        }
    }

    return false;
}

void logout(BIO *bio)
{
    string command = "A002 LOGOUT\r\n";
    if (BIO_write(bio, command.c_str(), command.length()) <= 0)
    {
        if (!BIO_should_retry(bio))
        {
            cerr << "Chyba při odesílání příkazu LOGOUT" << endl;
        }
    }
    else
    {
        char buffer[4096];
        int len = BIO_read(bio, buffer, 4096); // hardcoded buffer size
        if (len > 0)
        {
            buffer[len] = '\0';
            cerr << "Server response: " << buffer << endl;
        }
    }
}