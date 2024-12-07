#include "mail.h"
#include "commands.h"
#include ".env.h"

// connect to the imap server
imaps* createIMAPConnection() {
    try {
		imaps* conn = new imaps(IMAP_SERVER, 993);
        conn->authenticate(SERVICE_MAIL_ADDRESS, SERVICE_MAIL_PASSWORD, imaps::auth_method_t::LOGIN);
        return conn;
    }
    catch (imap_error& exc) {
        cout << exc.what() << endl;
    }
    catch (dialog_error& exc) {
        cout << exc.what() << endl;
    }
    exit(EXIT_FAILURE);
}

// get the latest unread message
bool getNewMessage(imaps& conn, message& msg) {
    try {
		// select the inbox folder
        conn.select("Inbox");

		// get the list of unread messages
        list<unsigned long> messages;
        list<imaps::search_condition_t> conds;
        conds.push_back(imaps::search_condition_t(imaps::search_condition_t::UNSEEN));
        conn.search(conds, messages, true);

        // fetch the latest unread message
        for (auto msg_uid : messages) {
            if (msg_uid <= 0) continue;

            conn.fetch("inbox", msg_uid, msg);

            return true;
        };
    }
    catch (imap_error& exc) {
        cout << exc.what() << endl;
    }
    catch (dialog_error& exc) {
        cout << exc.what() << endl;
    }

    return false;
}

string toLowerCase(string str) {
	transform(str.begin(), str.end(), str.begin(), ::tolower);
	return str;
}

// get the message text body
string getMessageTextBody(message& msg) {
	string body;

    if (msg.content_type().type == mailio::mime::media_type_t::MULTIPART) {
        for (const auto& part : msg.parts()) {
            if (part.content_type().type == mailio::mime::media_type_t::TEXT) {
				body = part.content();
                break;
            }
        }
    }
    else {
		body = msg.content();
    }

	return body;
}

bool receivedNewCommand(imaps& conn, string& title, string& nameObject, string& source, string& destination) {
    // mail message to store the fetched one
    message msg;

    // set the line policy to mandatory, so longer lines could be parsed
    msg.line_policy(codec::line_len_policy_t::MANDATORY);

	// if there is no new message, return false
    if (!getNewMessage(conn, msg)) return false;

	// get the message subject
	//title = toLowerCase(msg.subject());
	title = msg.subject();
    string body = getMessageTextBody(msg);

    if (
        title == START_APP || 
        title == STOP_APP || 
        title == START_SERVICE ||
        title == STOP_SERVICE ||
        title == SEND_FILE ||
        title == DELETE_FILE ||
        title == KEY_LOGGER
    ) {
		nameObject = body;
    }
    else if (title == COPY_FILE) {
        stringstream ss(body);
		getline(ss, source, '\n');
		getline(ss, destination, '\n');
    }
}