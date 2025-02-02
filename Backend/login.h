#ifndef LOGIN_H
#define LOGIN_H

char *handle_create_account_request(const char *request);

char *handle_login_request(const char *request, int *outUserId);

#endif
