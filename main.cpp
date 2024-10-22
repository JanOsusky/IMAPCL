#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <unistd.h>

#include "imap.h"
#include "connect.h"

// Bring standard library components into the current scope
using namespace std;

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
            return EXIT_FAILURE;
        }
    }

    if (optind < argc)
    {
        server = argv[optind];
    }

    if (server.empty() || authFile.empty() || outDir.empty())
    {
        printUsage();
        return EXIT_FAILURE;
    }

    string username, password;
    if (!parseAuthFile(authFile, username, password))
    {
        return EXIT_FAILURE;
    }

    // Print parsed arguments for debugging purposes
    cout << "Server: " << server << "\n";
    cout << "Port: " << port << "\n";
    cout << "Use TLS: " << (useTLS ? "Yes" : "No") << "\n";
    if (useTLS)
    {
        cout << "Cert File: " << certFile << "\n";
        cout << "Cert Dir: " << certDir << "\n";
    }
    cout << "Only new messages: " << (onlyNew ? "Yes" : "No") << "\n";
    cout << "Headers only: " << (headersOnly ? "Yes" : "No") << "\n";
    cout << "Mailbox: " << mailbox << "\n";
    cout << "Output Directory: " << outDir << "\n";
    cout << "Username: " << username << "\n";
    cout << "Password: (hidden)\n";
    // TODO: test TSL connections
    bio = connectToServer(server, port, useTLS, certFile, certDir, &ctx);

    if (bio == nullptr)
    {
        std::cerr << "Connection failed" << std::endl;
        return 1;
    }

    cout << "Connection successful to " << server << " on port " << port << endl;

    // Login to the server
    if (!login(bio, username, password))
    {
        cerr << "Login failed" << endl;
        return EXIT_FAILURE;
    }

    logout(bio);

    // Cleanup
    BIO_free_all(bio); // Frees both BIO and the associated SSL object
    if (ctx != nullptr)
    {
        SSL_CTX_free(ctx); // Free the SSL context
    }

    return EXIT_SUCCESS;
}
