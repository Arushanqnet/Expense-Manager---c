#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "home.h"

// ithu native json aa parase panna help pannuthu

// e.g. body = "{ \"type\":\"expense\",\"amount\":\"123.45\",\"date\":\"2023-10-21\",\"category\":\"Food\" }"
static void parse_body(const char *body, char *type, char *amount, char *date, char *category) {
    // This is an extremely naive parsing approach.
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

//Database table illaddi atha add panna use aahuthu
static int insert_into_db(const char *type, const char *amount, const char *date, const char *category, int user_id) {
    sqlite3 *db;
    char *err_msg = NULL;
    int rc;

    // 1. Database open pannuthu
    rc = sqlite3_open("transactions.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    // 2. Table create pannuthu user_id ooda (primary key)
    const char *create_table_sql =
        "CREATE TABLE IF NOT EXISTS transactions ("
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    user_id INTEGER,"
        "    trans_type TEXT,"
        "    amount REAL,"
        "    date TEXT,"
        "    category TEXT,"
        "    FOREIGN KEY(user_id) REFERENCES users(id)"
        ");";

    rc = sqlite3_exec(db, create_table_sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error creating table: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return rc;
    }

    // 3. Transaction insert panrathukku aana code
    char sql_insert[512];
    snprintf(sql_insert, sizeof(sql_insert),
             "INSERT INTO transactions (user_id, trans_type, amount, date, category) "
             "VALUES (%d, '%s', '%s', '%s', '%s');",
             user_id, type, amount, date, category);

    rc = sqlite3_exec(db, sql_insert, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL insert error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }

    // 4. database close pannuthu
    sqlite3_close(db);
    return rc;
}

char *handle_home_request(const char *request, int user_id) {
    // 1. hhtp request la irunthu body ya conver pannuthu
    const char *body_start = strstr(request, "\r\n\r\n");
    if (!body_start) {
        // No body found
        const char *response =
            "HTTP/1.1 400 Bad Request\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 12\r\n"
            "\r\n"
            "Bad Request";
        return strdup(response);
    }
    body_start += 4; // Move past "\r\n\r\n"

    // 2. Ovvoru transaction body ayum extract pannuthu
    char type[64] = {0};
    char amount[64] = {0};
    char date[64] = {0};
    char category[64] = {0};

    parse_body(body_start, type, amount, date, category);

    // 3. database la add panuthu user_id ooda
    int rc = insert_into_db(type, amount, date, category, user_id);

    // 4. response build pannuthu
    if (rc == SQLITE_OK) {
        const char *response_body = "Data inserted OK.";
        char response[256];
        snprintf(response, sizeof(response),
            "HTTP/1.1 200 OK\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: %zu\r\n"
            "\r\n"
            "%s",
            strlen(response_body), response_body);
        return strdup(response);
    } else {
        const char *response_body = "Database error occurred.";
        char response[256];
        snprintf(response, sizeof(response),
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: %zu\r\n"
            "\r\n"
            "%s",
            strlen(response_body), response_body);
        return strdup(response);
    }
}
