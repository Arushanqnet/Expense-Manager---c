#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "home.h"

// Helper to parse a naive JSON-like string: 
// e.g. body = "{ \"type\":\"expense\",\"amount\":\"123.45\",\"date\":\"2023-10-21\",\"category\":\"Food\" }"
static void parse_body(const char *body, char *type, char *amount, char *date, char *category) {
    // This is an extremely naive parsing approach.
    // In production, use a robust JSON library or at least safer string extraction.
    // We'll just find substrings between quotes.

    // For demonstration, we assume the keys are: type, amount, date, category
    // We'll do something like: strstr(body, "\"type\":\"") then read until next quote.
    const char *type_ptr = strstr(body, "\"type\":\"");
    if (type_ptr) {
        type_ptr += strlen("\"type\":\"");
        sscanf(type_ptr, "%[^\"]", type);
    }

    const char *amount_ptr = strstr(body, "\"amount\":\"");
    if (amount_ptr) {
        amount_ptr += strlen("\"amount\":\"");
        sscanf(amount_ptr, "%[^\"]", amount);
    }

    const char *date_ptr = strstr(body, "\"date\":\"");
    if (date_ptr) {
        date_ptr += strlen("\"date\":\"");
        sscanf(date_ptr, "%[^\"]", date);
    }

    const char *cat_ptr = strstr(body, "\"category\":\"");
    if (cat_ptr) {
        cat_ptr += strlen("\"category\":\"");
        sscanf(cat_ptr, "%[^\"]", category);
    }
}

// Create or open the database, create the table if not exists, insert the data
static int insert_into_db(const char *type, const char *amount, const char *date, const char *category) {
    sqlite3 *db;
    char *err_msg = NULL;
    int rc;

    // 1. Open (or create) the SQLite database
    rc = sqlite3_open("transactions.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    // 2. Create table if not exists
    const char *create_table_sql =
        "CREATE TABLE IF NOT EXISTS transactions ("
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    trans_type TEXT,"
        "    amount REAL,"
        "    date TEXT,"
        "    category TEXT"
        ");";

    rc = sqlite3_exec(db, create_table_sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error creating table: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return rc;
    }

    // 3. Build insert statement
    // **Potentially vulnerable** to SQL injection if used without prepared statements
    // For demonstration only
    char sql_insert[512];
    snprintf(sql_insert, sizeof(sql_insert),
             "INSERT INTO transactions (trans_type, amount, date, category) "
             "VALUES ('%s', '%s', '%s', '%s');",
             type, amount, date, category);

    rc = sqlite3_exec(db, sql_insert, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL insert error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }

    // 4. Close database
    sqlite3_close(db);
    return rc;
}

char *handle_home_request(const char *request) {
    // 1. Extract request body from the raw HTTP request
    // A naive approach to find the blank line and then everything after it
    const char *body_start = strstr(request, "\r\n\r\n");
    if (!body_start) {
        // No body found
        const char *response =
            "HTTP/1.1 400 Bad Request\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 12\r\n"
            "\r\n"
            "Bad Request";
        char *res_copy = (char *)malloc(strlen(response) + 1);
        strcpy(res_copy, response);
        return res_copy;
    }
    body_start += 4; // Move past "\r\n\r\n"

    // 2. Parse the body to extract type, amount, date, category
    char type[64] = {0};
    char amount[64] = {0};
    char date[64] = {0};
    char category[64] = {0};

    parse_body(body_start, type, amount, date, category);

    // 3. Insert into the database
    int rc = insert_into_db(type, amount, date, category);

    // 4. Construct a response
    if (rc == SQLITE_OK) {
        const char *response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 19\r\n"
            "\r\n"
            "Data inserted OK.";
        char *res_copy = (char *)malloc(strlen(response) + 1);
        strcpy(res_copy, response);
        return res_copy;
    } else {
        const char *response =
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 21\r\n"
            "\r\n"
            "Database error occurred.";
        char *res_copy = (char *)malloc(strlen(response) + 1);
        strcpy(res_copy, response);
        return res_copy;
    }
}
