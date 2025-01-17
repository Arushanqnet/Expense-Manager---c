#ifndef HOME_H
#define HOME_H

// Handle the /home POST request, storing the transaction 
// associated with the given user_id
char *handle_home_request(const char *request, int user_id);

#endif
