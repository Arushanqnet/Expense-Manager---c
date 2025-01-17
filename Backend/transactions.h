#ifndef TRANSACTIONS_H
#define TRANSACTIONS_H

// Returns a malloc'd JSON string of all transactions belonging to user_id.
// Caller must free the returned string.
char* handle_get_transactions_request(int user_id);

#endif