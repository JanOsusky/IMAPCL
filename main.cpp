/**
 * @file main.cpp
 * @brief Main entry point for the IMAPCL project.
 * 
 * This file contains the main function and related utility functions for the IMAPCL project.
 * The program connects to an IMAP server, authenticates using provided credentials, and fetches emails.
 * 
 * @project IMAPCL
 * @date 2024-10-24
 * @author Jan Osuský
 * @login xosusk00
 */
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <unistd.h>
#include <regex>

#include "imap.h"
#include "connect.h"

// Bring standard library components into the current scope
using namespace std;
#include <fstream>
#include <string>
#include <map>
#include <unistd.h>
#include <regex>

#include "imap.h"
#include "connect.h"

// Bring standard library components into the current scope
using namespace std;

// Function to print the usage of the program
void printUsage()
{
    cout << "\n--Použití: imapcl server [-p port] [-T [-c certfile] [-C certaddr]] [-n] [-h] -a auth_file [-b MAILBOX] -o out_dir\n\n\n";
    cout << "--Pořadí parametrů je libovolné. Popis parametrů:\n\n\n";
    cout << "-Povinně je uveden název serveru (IP adresa, nebo doménové jméno) požadovaného zdroje.\n";
    cout << "-Volitelný parametr -p port specifikuje číslo portu na serveru. Zvolte vhodnou výchozí hodnotu v závislosti na specifikaci parametru -T a číslech portů registrovaných organizací IANA.\n";
    cout << "-Parametr -T zapíná šifrování (imaps), pokud není parametr uveden, použije se nešifrovaná varianta protokolu.\n";
    cout << "-Volitelný parametr -c s oubor s certifikáty, který se použije pro ověření platnosti certifikátu SSL/TLS předloženého serverem.\n";
    cout << "-Volitelný parametr -C určuje adresář, ve kterém se mají vyhledávat certifikáty, které se použijí pro ověření platnosti certifikátu SSL/TLS předloženého serverem. Výchozí hodnota je /etc/ssl/certs.\n";
    cout << "-Při použití parametru -n se bude pracovat (číst) pouze s novými zprávami.\n\n";
    cout << "-Při použití parametru -h se budou stahovat pouze hlavičky zpráv.\n\n";
    cout << "-Povinný parametr -a auth_file odkazuje na soubor s autentizaci (příkaz LOGIN), obsah konfiguračního souboru auth_file je zobrazený níže.\n";
    cout << "-Parametr -b specifikuje název schránky, se kterou se bude na serveru pracovat. Výchozí hodnota je INBOX.\n";
    cout << "-Povinný parametr -o out_dir specifikuje výstupní adresář, do kterého má program stažené zprávy uložit.\n";
}
// Function to trim whitespace from a string
 std::string trim(const std::string& str) {
    return std::regex_replace(str, std::regex("^\\s+|\\s+$"), "");
}
// Function to parse the authentication file
bool parseAuthFile(const string &authFile, string &username, string &password)
{
    ifstream file(authFile);
    if (!file)
    {
        cerr << "Chyba: Nelze otevřít soubor s autentizací: " << authFile << endl;
        return false;
    }

    string line;
    while (getline(file, line))
    {
        size_t delim = line.find("=");
        if (delim != string::npos)
        {
            string key = line.substr(0, delim);
            string value = line.substr(delim + 1);
            key = trim(key);
            value = trim(value);
           
            if (key == "username")
            {
                username = value;
            }
            else if (key == "password")
            {
                password = value;
            }
        }
    }
    // Check if both username and password were found
    if (username.empty() || password.empty())
    {
        cerr << "Error: Missing username or password in auth file\n";
        return false;
    }

    return true;
}

int main(int argc, char *argv[])
{
    map<string, string> args;
    string server;
    string port = "143"; // Default IMAP port
    bool useTLS = false;
    string certFile;
    string certDir = "/etc/ssl/certs";
    bool onlyNew = false;
    bool headersOnly = false;
    string authFile;
    string mailbox = "INBOX";
    string outDir;
    BIO *bio = nullptr;
    SSL_CTX *ctx = nullptr;

    int opt;
    // Parse command line arguments
    while ((opt = getopt(argc, argv, "p:Tc:C:nha:b:o:")) != -1)
    {
        switch (opt)
        {
        case 'p':
            port = optarg;
            break;
        case 'T':
            useTLS = true;
            port = "993"; // Default IMAP port when TLS is enabled
            break;
        case 'c':
            certFile = optarg;
            break;
        case 'C':
            certDir = optarg;
            break;
        case 'n':
            onlyNew = true;
            break;
        case 'h':
            headersOnly = true;
            break;
        case 'a':
            authFile = optarg;
            break;
        case 'b':
            mailbox = optarg;
            break;
        case 'o':
            outDir = optarg;
            break;
        default:
            printUsage();
            return -1;
        }
    }

    if (optind < argc)
    {
        server = argv[optind];
    }
    // Check if required arguments are present
    if (server.empty() || authFile.empty() || outDir.empty())
    {
        printUsage();
        return -1;
    }

    string username, password;
    if (!parseAuthFile(authFile, username, password))
    {
        return -1;
    }

    // Initialize the OpenSSL library
    bio = connectToServer(server, port, useTLS, certFile, certDir, &ctx);

    if (bio == nullptr)
    {
        std::cerr << "Připojení selhalo" << std::endl;
        return 1;
    }
    
    // Login to the server
    if (!login(bio, username, password))
    {
        cerr << "Přihlášení selhalo" << endl;
        return -1;
    }

    // Fetch mail from the server
    int result = fetchMail(bio, mailbox, outDir, onlyNew, headersOnly);
    
    // Print the result
    cout << "Staženo " << result << (headersOnly ? " hlaviček" : "") <<  (onlyNew ? " nových zpráv " : " zpráv ") << "ze schránky " << mailbox <<  " do složky " << outDir <<   endl;

    // Logout from the server
    logout(bio);

    // Cleanup
    BIO_free_all(bio); // Frees both BIO and the associated SSL object
    if (ctx != nullptr)
    {
        SSL_CTX_free(ctx); // Free the SSL context
    }

    return EXIT_SUCCESS;
}
