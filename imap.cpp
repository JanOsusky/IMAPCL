#include "imap.h"

bool login(BIO *bio, string username, string password)
{
    string command = "A002 LOGIN " + username + " " + password + "\r\n";
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
        BIO_read(bio, buffer, sizeof(buffer) - 1);
        string strResponse(buffer);
        if(strResponse.find("A002 OK") == string::npos) {
            return false;
        } else {
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

      // Check if the directory already exists
    if (filesystem::exists(outDir)) {
        cout << "Output directory already exists: " << outDir << endl;
    } else {
        // Attempt to create the directory
        if (!filesystem::create_directory(outDir)) {
            cerr << "Error creating output directory: " << outDir << endl;
            return -1;
        }
        cout << "Output directory created: " << outDir << endl;
    }

    // Step 1: Select the mailbox
    string select_cmd = "A003 SELECT \"" + mailbox + "\"\r\n";
    BIO_puts(bio, select_cmd.c_str());

    // Read server response (optional, but good for error handling)
    char response[32768];
    BIO_read(bio, response, sizeof(response) - 1);
    string strResponse(response);
    if(strResponse.find("A003 OK") == string::npos) {
        cerr << "Error selecting mailbox: " << mailbox << endl;
        return -1;
    }

    // Step 2: Search for messages (we can search for new messages)
    string search_cmd = onlyNew ? "A004 SEARCH UNSEEN\r\n" : "A004 SEARCH ALL\r\n";
    BIO_puts(bio, search_cmd.c_str());

    // Read search response
    BIO_read(bio, response, sizeof(response) - 1);
    response[sizeof(response) - 1] = '\0';
    cout << "Search Response: " << response << endl;

    // Example: Assume we parse the response to get message IDs
    vector<string> message_ids; // Populate this with the message IDs
    // For demonstration, let's assume we got some message IDs
    message_ids.push_back("1");
    message_ids.push_back("2");

    // Step 3: Fetch emails using FETCH command
    int downloaded_count = 0;
    for (const auto &id : message_ids)
    {
        string fetch_cmd = "A00 FETCH " + id + (headersOnly ? " BODY.PEEK[HEADER]" : " BODY[]") + "\r\n";
        BIO_puts(bio, fetch_cmd.c_str());

        // Read fetch response
        BIO_read(bio, response, sizeof(response) - 1);
        response[sizeof(response) - 1] = '\0';

        // Step 4: Save the email content to a file
        string filename = outDir + "/email_" + id + (headersOnly ? ".header" : ".eml");
        FILE *file = fopen(filename.c_str(), "w");
        if (file)
        {
            fputs(response, file);
            fclose(file);
            downloaded_count++;
        }
        else
        {
            cerr << "Error saving email " << id << " to file." << endl;
        }
    }

    return downloaded_count; // Return the count of downloaded messages
}

