#ifndef LOGIN_H
#define LOGIN_H

// Creates a user account
char *handle_create_account_request(const char *request);

// Logs in a user.
// - If login is successful, sets *outUserId to the user's db id.
// - Returns a message string to send back to the client.
//   Caller must free() the returned string.
char *handle_login_request(const char *request, int *outUserId);

#endif
