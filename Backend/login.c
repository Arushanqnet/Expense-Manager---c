#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

#include "login.h"

// Sadharana JSON-mathiri body parser. 
// Request body-ai ethirpaarkum: { "username": "bob", "password": "secret" }
static void parse_user_body(const char *body, char *username, char *password) {
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

// 'users' table illainaal create pannum
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

// Create account-aa handle pannum.
char *handle_create_account_request(const char *request) {
    // 1. Body-yai eduthukol
    const char *body_start = strstr(request, "\r\n\r\n");
    if (!body_start) {
        return strdup("Bad Request: No body found");
    }
    body_start += 4;

    // 2. username/password-ai parse pannum
    char username[128] = {0};
    char password[128] = {0};
    parse_user_body(body_start, username, password);

    // 3. DB-a thirakkum
    sqlite3 *db;
    int rc = sqlite3_open("transactions.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open DB: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return strdup("Database error");
    }

    // 4. 'users' table irukkirathu ena urudhi pannum
    rc = ensure_users_table(db);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return strdup("Failed to create users table.");
    }

    // 5. user-ai insert pannum
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
        return strdup("Error creating account (username may be taken).");
    }

    // 6. DB-a moodyum
    sqlite3_close(db);

    // 7. vetri-yai thiruppi kodukkum
    return strdup("Account created successfully.");
}

// SELECT query-il irundhu user-in ID-ai eduthukkum callback
static int user_id_callback(void *data, int argc, char **argv, char **colName) {
    int *pUserId = (int *)data;
    if (argv[0]) {
        *pUserId = atoi(argv[0]);
    }
    return 0;
}

// login-ai handle pannum
// vetriyaga irundhaal, user.id-ai petru *outUserId-il vaippom
char *handle_login_request(const char *request, int *outUserId) {
    // 1. body-yai eduthukol
    const char *body_start = strstr(request, "\r\n\r\n");
    if (!body_start) {
        return strdup("Bad Request: No body found");
    }
    body_start += 4;

    // 2. user info-ai parse pannum
    char username[128] = {0};
    char password[128] = {0};
    parse_user_body(body_start, username, password);

    // 3. DB-a thirakkum
    sqlite3 *db;
    int rc = sqlite3_open("transactions.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open DB: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return strdup("Database error");
    }

    // 4. 'users' table irukkirathu ena urudhi pannum
    rc = ensure_users_table(db);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return strdup("Failed to create/check users table.");
    }

    // 5. user-in ID-ai eduthukkum
    // Row irundhaal, adhan ID-ai kidaikkum. Ilainaal 0 kidaikkum.
    char sql_query[512];
    snprintf(sql_query, sizeof(sql_query),
             "SELECT id FROM users WHERE username='%s' AND password='%s' LIMIT 1;",
             username, password);

    *outUserId = 0;  // default to 0 (not found)
    char *err_msg = NULL;
    rc = sqlite3_exec(db, sql_query, user_id_callback, outUserId, &err_msg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error checking login: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return strdup("Error checking login");
    }

    sqlite3_close(db);

    // 6. vetriyaga illainaal tholvi-yaga thiruppi kodukkum
    if (*outUserId > 0) {
        // Found the user
        char msg[128];
        snprintf(msg, sizeof(msg), "Login successful! Your user ID is %d.", *outUserId);
        return strdup(msg);
    } else {
        return strdup("Invalid username or password.");
    }
}
