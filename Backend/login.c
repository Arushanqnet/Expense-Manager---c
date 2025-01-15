#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

#include "login.h"

// Naive JSON-like body parser. 
// We assume the request body is something like:
// {
//   "username": "bob",
//   "password": "mypassword"
// }
static void parse_user_body(const char *body, char *username, char *password) {
    // Very naive approach. Use robust JSON libraries for real production code.
    const char *u_ptr = strstr(body, "\"username\"");
    if (u_ptr) {
        u_ptr = strstr(u_ptr, ":");
        if (u_ptr) {
            u_ptr = strstr(u_ptr, "\"");
            if (u_ptr) {
                u_ptr++;
                sscanf(u_ptr, "%[^\"]", username);
            }
        }
    }

    const char *p_ptr = strstr(body, "\"password\"");
    if (p_ptr) {
        p_ptr = strstr(p_ptr, ":");
        if (p_ptr) {
            p_ptr = strstr(p_ptr, "\"");
            if (p_ptr) {
                p_ptr++;
                sscanf(p_ptr, "%[^\"]", password);
            }
        }
    }
}

// Creates a 'users' table if it doesn't exist
static int ensure_users_table(sqlite3 *db) {
    const char *create_sql = 
        "CREATE TABLE IF NOT EXISTS users ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " username TEXT UNIQUE,"
        " password TEXT"
        ");";
    
    char *err_msg = NULL;
    int rc = sqlite3_exec(db, create_sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error creating users table: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
    return rc;
}

// Handle create account
// 1) Parse username + password
// 2) Insert into DB
char *handle_create_account_request(const char *request) {
    // 1. Extract body
    const char *body_start = strstr(request, "\r\n\r\n");
    if (!body_start) {
        // No body
        return strdup("Bad Request: No body found");
    }
    body_start += 4;

    // 2. Parse out username/password
    char username[128] = {0};
    char password[128] = {0};
    parse_user_body(body_start, username, password);

    // 3. Open DB
    sqlite3 *db;
    int rc = sqlite3_open("transactions.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open DB: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return strdup("Database error");
    }

    // 4. Ensure 'users' table
    rc = ensure_users_table(db);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return strdup("Failed to create users table.");
    }

    // 5. Insert user (naive approach; do prepared statements in production)
    char sql_insert[512];
    snprintf(sql_insert, sizeof(sql_insert),
             "INSERT INTO users (username, password) VALUES ('%s', '%s');",
             username, password);

    char *err_msg = NULL;
    rc = sqlite3_exec(db, sql_insert, NULL, NULL, &err_msg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        // Possibly the user already exists, or some other insertion error
        return strdup("Error creating account (username may be taken).");
    }

    // 6. Close DB
    sqlite3_close(db);

    // 7. Return success
    return strdup("Account created successfully.");
}

// This is our callback function to parse the result of the COUNT(*) query.
static int userExistsCallback(void *data, int argc, char **argv, char **colName) {
    // data is a pointer to our 'user_exists' int.
    int *pExists = (int *)data;
    if (argv[0]) {
        *pExists = atoi(argv[0]); // Convert the COUNT(*) result to int
    }
    return 0; // Return 0 to indicate success to SQLite
}

// Handle login
// 1) Parse username/password
// 2) Check if user matches in DB
char *handle_login_request(const char *request) {
    // 1. Extract body
    const char *body_start = strstr(request, "\r\n\r\n");
    if (!body_start) {
        return strdup("Bad Request: No body found");
    }
    body_start += 4;

    // 2. Parse user info
    char username[128] = {0};
    char password[128] = {0};
    parse_user_body(body_start, username, password);

    // 3. Open DB
    sqlite3 *db;
    int rc = sqlite3_open("transactions.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open DB: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return strdup("Database error");
    }

    // 4. Ensure 'users' table
    rc = ensure_users_table(db);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return strdup("Failed to create/check users table.");
    }

    // 5. Check user in DB
    //    For security, do prepared statements in production.
    char sql_query[512];
    snprintf(sql_query, sizeof(sql_query),
             "SELECT COUNT(*) FROM users WHERE username='%s' AND password='%s';",
             username, password);

    int user_exists = 0;
    char *err_msg = NULL;
    rc = sqlite3_exec(db, sql_query,
                      userExistsCallback, // <-- static callback function
                      &user_exists,
                      &err_msg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error checking login: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return strdup("Error checking login");
    }

    sqlite3_close(db);

    // 6. Return success or failure
    if (user_exists > 0) {
        return strdup("Login successful!");
    } else {
        return strdup("Invalid username or password.");
    }
}
