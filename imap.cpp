#include "imap.h"

bool login(BIO *bio, string username, string password)
{
    string command = "A001 LOGIN " + username + " " + password + "\r\n";
    if (BIO_puts(bio, command.c_str()) <= 0)
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
    if (BIO_puts(bio, command.c_str()) <= 0)
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

int fetchMail(BIO *bio, string mailbox, string outDir, bool onlyNew, bool headersOnly)
{
    string command = "A003 SELECT " + mailbox + "\r\n";
    if (BIO_puts(bio, command.c_str()) <= 0)
    {
        if (!BIO_should_retry(bio))
        {
            cerr << "Chyba při odesílání příkazu SELECT" << endl;
            return -1;
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

    return 0;

    // Ensure output directory exists
    filesystem::create_directory(outDir);

    // Step 1: Select the mailbox
    string select_cmd = "A003 SELECT \"" + mailbox + "\"\r\n";
    BIO_puts(bio, select_cmd.c_str());

    // Read server response (optional, but good for error handling)
    char response[1024];
    BIO_read(bio, response, sizeof(response) - 1);
    response[sizeof(response) - 1] = '\0';
    std::cout << "Server Response: " << response << std::endl;

    // Step 2: Search for messages (we can search for new messages)
    std::string search_cmd = onlyNew ? "A002 SEARCH UNSEEN\r\n" : "A002 SEARCH ALL\r\n";
    BIO_puts(bio, search_cmd.c_str());

    // Read search response
    BIO_read(bio, response, sizeof(response) - 1);
    response[sizeof(response) - 1] = '\0';
    std::cout << "Search Response: " << response << std::endl;

    // Example: Assume we parse the response to get message IDs
    std::vector<std::string> message_ids; // Populate this with the message IDs
    // For demonstration, let's assume we got some message IDs
    message_ids.push_back("1");
    message_ids.push_back("2");

    // Step 3: Fetch emails using FETCH command
    int downloaded_count = 0;
    for (const auto &id : message_ids)
    {
        std::string fetch_cmd = "A003 FETCH " + id + (headers_only ? " BODY.PEEK[HEADER]" : " BODY[]") + "\r\n";
        BIO_puts(bio, fetch_cmd.c_str());

        // Read fetch response
        BIO_read(bio, response, sizeof(response) - 1);
        response[sizeof(response) - 1] = '\0';

        // Step 4: Save the email content to a file
        std::string filename = out_dir + "/email_" + id + (headers_only ? ".header" : ".eml");
        FILE *file = fopen(filename.c_str(), "w");
        if (file)
        {
            fputs(response, file);
            fclose(file);
            downloaded_count++;
        }
        else
        {
            std::cerr << "Error saving email " << id << " to file." << std::endl;
        }
    }

    return downloaded_count; // Return the count of downloaded messages
}
