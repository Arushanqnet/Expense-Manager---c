#ifndef LOGIN_H
#define LOGIN_H

/**
 * Handle creating a new user account.
 * @param request The raw HTTP request string with body containing username/password.
 * @return Dynamically allocated string containing success or error message.
 *         The caller must free() the returned string.
 */
char *handle_create_account_request(const char *request);

/**
 * Handle user login.
 * @param request The raw HTTP request string with body containing username/password.
 * @return Dynamically allocated string containing success or error message.
 *         The caller must free() the returned string.
 */
char *handle_login_request(const char *request);

#endif
