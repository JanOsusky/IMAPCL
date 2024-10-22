#include "connect.h"

using namespace std;

// Initialize OpenSSL library
void InitializeOpenSSL()
{
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
}

BIO *connectToServer(string server, string port, bool useTLS, string certFile, string certDir, SSL_CTX **ctx)
{

    BIO *bio = nullptr;
    *ctx = nullptr;
    SSL *ssl = nullptr;
    string credentials = server + ":" + port;

    // Initialize connection with the server using TLS
    if (useTLS)
    {
        *ctx = SSL_CTX_new(SSLv23_client_method());
        if (certFile == "")
        {

            if (!SSL_CTX_load_verify_locations(*ctx, NULL, certDir.c_str()))
            {
                cerr << "Chyba při načítání certifikátu" << endl;
                return nullptr;
            }
        }
        else
        {

            if (!SSL_CTX_load_verify_locations(*ctx, certFile.c_str(), NULL))
            {
                cerr << "Chyba při načítání certifikátu" << endl;
                return nullptr;
            }
        }
        bio = BIO_new_ssl_connect(*ctx);
        BIO_get_ssl(bio, &ssl);
        SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY); // Automatic new handshake if needed

        // Attempt to connect to the server
        BIO_set_conn_hostname(bio, credentials.c_str());
        if (SSL_get_verify_result(ssl) != X509_V_OK)
        {
            cerr << "Chyba ověření certifikátu" << endl;
            return nullptr; // TODO: mozna by to nebylo nutne vratit nullptr a klidne pokracovat nevim
        }
    }
    else // Initialize connection with the server without TLS
    {
        bio = BIO_new_connect(credentials.c_str());
    }

    if (bio == nullptr || BIO_do_connect(bio) <= 0)
    {
        std::cerr << "Připojení selhalo" << std::endl;
        return nullptr;
    }

    char buffer[4096];
    int len = BIO_read(bio, buffer, 4096); // hardcoded buffer size
    if (len > 0)
    {
        buffer[len] = '\0';
        cerr << "Server response: " << buffer << endl;
    }

    return bio; // Return the bio object
}
