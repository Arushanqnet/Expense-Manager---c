#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include "transactions.h"

char* handle_get_transactions_request(int user_id) {
    sqlite3 *db;
    sqlite3_stmt *res;
    int rc;

    rc = sqlite3_open("transactions.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        // Return some JSON-encoded error message
        char *error_msg = strdup("{\"error\": \"Cannot open database\"}");
        return error_msg;
    }

    // Query only the columns we want: id, trans_type, amount, date, category
    const char *sql = "SELECT id, trans_type, amount, date, category "
                      "FROM transactions "
                      "WHERE user_id = ?";

    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        char *error_msg = strdup("{\"error\": \"Failed to prepare statement\"}");
        return error_msg;
    }

    // Bind the user_id parameter
    sqlite3_bind_int(res, 1, user_id);

    // Build a JSON array string of transactions
    // Example structure: [{"id":1,"trans_type":"credit","amount":1000.50,"date":"2025-01-01","category":"Salary"}, ...]
    char *json_result = strdup("[");
    int first_record = 1;

    while ((rc = sqlite3_step(res)) == SQLITE_ROW) {
        if (!first_record) {
            // Add a comma separator if this isn’t the first record
            json_result = realloc(json_result, strlen(json_result) + 2);
            strcat(json_result, ",");
        }
        first_record = 0;

        int id = sqlite3_column_int(res, 0);
        const unsigned char *trans_type_raw = sqlite3_column_text(res, 1);
        double amount = sqlite3_column_double(res, 2);
        const unsigned char *date_raw = sqlite3_column_text(res, 3);
        const unsigned char *category_raw = sqlite3_column_text(res, 4);

        // Use safe string fallback if any column is NULL
        const char *trans_type_str = trans_type_raw ? (const char*)trans_type_raw : "";
        const char *date_str       = date_raw       ? (const char*)date_raw       : "";
        const char *category_str   = category_raw   ? (const char*)category_raw   : "";

        // Prepare a temporary buffer for one transaction object
        char transaction_buffer[512];
        snprintf(transaction_buffer, sizeof(transaction_buffer),
            "{\"id\":%d,\"trans_type\":\"%s\",\"amount\":%.2f,\"date\":\"%s\",\"category\":\"%s\"}",
            id, trans_type_str, amount, date_str, category_str);

        // Extend json_result and append
        size_t new_length = strlen(json_result) + strlen(transaction_buffer) + 1;
        json_result = realloc(json_result, new_length + 1); // +1 for '\0'
        strcat(json_result, transaction_buffer);
        printf("json_result: %s\n", json_result);
    }

    // Close the bracket for JSON array
    size_t final_len = strlen(json_result) + 2; // for ']' and '\0'
    json_result = realloc(json_result, final_len);
    strcat(json_result, "]");

    // Clean up
    sqlite3_finalize(res);
    sqlite3_close(db);

    return json_result;
}