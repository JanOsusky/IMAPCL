#include "imap.h"

// Function to log in to the server
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

// Function to log out from the server
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
        BIO_read(bio, buffer, sizeof(buffer) - 1);
    }
}

// Function to trim whitespace from both ends of a string
string trim(string s) {
   size_t start = s.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) return ""; 

    size_t end = s.find_last_not_of(" \t\n\r\f\v"); 
    return s.substr(start, end - start + 1);
}

int fetchMail(BIO *bio, string mailbox, string outDir, bool onlyNew, bool headersOnly)
{

    // Check if the directory already exists
    if (!filesystem::exists(outDir)) {
        if (!filesystem::create_directory(outDir)) {
            cerr << "Error creating output directory: " << outDir << endl;
            return -1;
        }
    }
   

    // Select the mailbox
    string select_cmd = "A003 SELECT \"" + mailbox + "\"\r\n";
    BIO_puts(bio, select_cmd.c_str());

    // Read server response (optional, but good for error handling)
    char response[8192];
    BIO_read(bio, response, sizeof(response) - 1);
    string strResponse(response);
    if(strResponse.find("A003 OK") == string::npos) {
        cerr << "Error selecting mailbox: " << mailbox << endl;
        return -1;
    }

    // Search for messages in the mailbox, either all or only unseen
    string search_cmd = onlyNew ? "A004 SEARCH UNSEEN\r\n" : "A004 SEARCH ALL\r\n";
    BIO_puts(bio, search_cmd.c_str());

    // Read search response
    vector<string> message_ids; // Vector to store the message IDs
    BIO_read(bio, response, sizeof(response) - 1);
    strResponse.clear();
    strResponse.append(response);
    if (strResponse.find("* SEARCH") != string::npos && strResponse.find("* SEARCH\r\n") == string::npos) {
      // Use istringstream to split the response line by line
    istringstream responseStream(strResponse);
    string line;
    while (getline(responseStream, line)) {
        if (line.find("* SEARCH") != string::npos) {
            // Find the starting position of the message IDs
            size_t start = line.find("SEARCH") + strlen("SEARCH ");

            // Extract the substring that contains the message IDs
            string idsString = line.substr(start);

            // Use stringstream to split the IDs by spaces
            stringstream idsStream(idsString);
            string id;
            while (idsStream >> id) {
                id = trim(id);
                message_ids.push_back(id);
            }
            break; // We only need to process the first * SEARCH line
        }
    }
    } else {
        if(strResponse.find("* SEARCH\r\n") == string::npos)
        {
            cerr << "Error in search response format." << endl;
            return -1;
        }
    }




    // Fetch emails using FETCH command
    int downloaded_count = 0;
    for (const auto &id : message_ids)
    {
        string fetch_cmd = "A00 FETCH " + id + (headersOnly ? " BODY.PEEK[HEADER]" : " BODY[]") + "\r\n";
        BIO_puts(bio, fetch_cmd.c_str());


        string email;
        bool fetch_completed = false;

        while (!fetch_completed)
        {
            // Read fetch response
            int len = BIO_read(bio, response, sizeof(response) - 1);
            response[len] = '\0';
            email.append(response);

            // Check if the response contains the end of the email
            if (strstr(response, "A00 OK") != nullptr)
            {
                fetch_completed = true;
            }
        }

        // Save the email content to a file
        string filename = outDir + "/email_" + id + (headersOnly ? ".header" : ".eml");
        FILE *file = fopen(filename.c_str(), "w");
        if (file)
        {
            fputs(email.c_str(), file);
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

