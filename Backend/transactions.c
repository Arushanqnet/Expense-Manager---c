/******************************************************************************
 * transactions.c
 *
 * Compile together with your main.c, home.c, login.c:
 *   gcc -o server main.c home.c login.c transactions.c -lsqlite3
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>

// -------------------------------------------------------------------
// HELPER: Convert a 2-digit month string ("01".."12") to a name ("January".."December").
static const char* month_number_to_name(const char* monthNum)
{
    if (strcmp(monthNum, "01") == 0) return "January";
    if (strcmp(monthNum, "02") == 0) return "February";
    if (strcmp(monthNum, "03") == 0) return "March";
    if (strcmp(monthNum, "04") == 0) return "April";
    if (strcmp(monthNum, "05") == 0) return "May";
    if (strcmp(monthNum, "06") == 0) return "June";
    if (strcmp(monthNum, "07") == 0) return "July";
    if (strcmp(monthNum, "08") == 0) return "August";
    if (strcmp(monthNum, "09") == 0) return "September";
    if (strcmp(monthNum, "10") == 0) return "October";
    if (strcmp(monthNum, "11") == 0) return "November";
    if (strcmp(monthNum, "12") == 0) return "December";
    return "Unknown";
}

// -------------------------------------------------------------------
// METHOD 1 DATA: Return a raw JSON array of individual transactions
// (suitable for a “normal table” in your frontend).
//
// Example of final JSON array:
//   [
//     {
//       "id": 6, "trans_type": "expense", "amount": 200.00,
//       "date": "2025-01-17", "category": "Transport"
//     },
//     ...
//   ]
// -------------------------------------------------------------------
static char* get_transactions_raw_list(int user_id)
{
    sqlite3 *db;
    sqlite3_stmt *res;
    int rc;

    // 1) Open the database
    rc = sqlite3_open("transactions.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return strdup("{\"error\":\"Cannot open database\"}");
    }

    // 2) Prepare a query to select the needed columns
    const char *sql =
        "SELECT id, trans_type, amount, date, category "
        "FROM transactions "
        "WHERE user_id = ? "
        "ORDER BY date DESC;";

    rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return strdup("{\"error\":\"Failed to prepare statement\"}");
    }

    // 3) Bind the user_id
    sqlite3_bind_int(res, 1, user_id);

    // 4) Build a JSON array string
    //    [
    //      {"id":..., "trans_type":"...", "amount":..., "date":"...", "category":"..."},
    //      ...
    //    ]
    char *json_result = strdup("[");
    int first_record = 1;

    while ((rc = sqlite3_step(res)) == SQLITE_ROW) {
        if (!first_record) {
            // Add comma between objects
            json_result = realloc(json_result, strlen(json_result) + 2);
            strcat(json_result, ",");
        }
        first_record = 0;

        // Extract each column
        int id = sqlite3_column_int(res, 0);
        const unsigned char *trans_type_raw = sqlite3_column_text(res, 1);
        double amount = sqlite3_column_double(res, 2);
        const unsigned char *date_raw = sqlite3_column_text(res, 3);
        const unsigned char *category_raw = sqlite3_column_text(res, 4);

        // Fallback if columns are NULL
        const char *trans_type = trans_type_raw ? (const char*)trans_type_raw : "";
        const char *date       = date_raw       ? (const char*)date_raw       : "";
        const char *category   = category_raw   ? (const char*)category_raw   : "";

        // Format a JSON object for this row
        char row_buffer[512];
        snprintf(row_buffer, sizeof(row_buffer),
                 "{\"id\":%d,\"trans_type\":\"%s\",\"amount\":%.2f,\"date\":\"%s\",\"category\":\"%s\"}",
                 id, trans_type, amount, date, category);

        // Append to the JSON array
        size_t new_len = strlen(json_result) + strlen(row_buffer) + 1;
        json_result = realloc(json_result, new_len + 1); // +1 for '\0'
        strcat(json_result, row_buffer);
    }

    // 5) Close the array
    size_t final_len = strlen(json_result) + 2;  // for ']' + '\0'
    json_result = realloc(json_result, final_len);
    strcat(json_result, "]");

    // 6) Clean up
    sqlite3_finalize(res);
    sqlite3_close(db);

    return json_result;
}

// -------------------------------------------------------------------
// METHOD 2 DATA: Return a Chart.js "bar" config object with dynamic
// monthly sums for "expense" vs "income" based on the current user's data.
//
// We'll produce a JSON object like:
//   {
//     "type": "bar",
//     "data": {
//       "labels": ["January","February",...,"December"],
//       "datasets": [
//         {
//           "label": "Expenses",
//           "data": [ sum_for_Jan, sum_for_Feb, ... , sum_for_Dec ],
//           "backgroundColor": "...",
//           ...
//         },
//         {
//           "label": "Income",
//           "data": [ ... ],
//           ...
//         }
//       ]
//     },
//     "options": {
//       "scales": { "y": { "beginAtZero": true } }
//     }
//   }
// -------------------------------------------------------------------
static char* get_barchart_config(int user_id)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;

    // We'll accumulate sums for each month (01..12)
    double expenses[12] = {0};
    double income[12]   = {0};

    // 1) Open the database
    rc = sqlite3_open("transactions.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return strdup("{\"error\":\"Cannot open database\"}");
    }

    // 2) Summation query for expenses by month
    const char *sql_expenses =
        "SELECT strftime('%m', date) AS month_num, SUM(amount) "
        "FROM transactions "
        "WHERE user_id = ? AND trans_type = 'expense' "
        "GROUP BY strftime('%Y-%m', date) "
        "ORDER BY month_num;";

    rc = sqlite3_prepare_v2(db, sql_expenses, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement (expenses): %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return strdup("{\"error\":\"Failed to prepare statement for expenses\"}");
    }

    // Bind user_id for expenses
    sqlite3_bind_int(stmt, 1, user_id);

    // Read the sums into our expenses array
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const unsigned char *month_num = sqlite3_column_text(stmt, 0);
        double sum_amount = sqlite3_column_double(stmt, 1);
        if (month_num) {
            // month_num will be "01","02",...,"12" => convert to index 0..11
            int index = atoi((const char *)month_num) - 1;
            if (index >= 0 && index < 12) {
                expenses[index] = sum_amount;
            }
        }
    }
    sqlite3_finalize(stmt);

    // 3) Summation query for income by month
    const char *sql_income =
        "SELECT strftime('%m', date) AS month_num, SUM(amount) "
        "FROM transactions "
        "WHERE user_id = ? AND trans_type = 'income' "
        "GROUP BY strftime('%Y-%m', date) "
        "ORDER BY month_num;";

    rc = sqlite3_prepare_v2(db, sql_income, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement (income): %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return strdup("{\"error\":\"Failed to prepare statement for income\"}");
    }

    // Bind user_id for income
    sqlite3_bind_int(stmt, 1, user_id);

    // Read the sums into our income array
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const unsigned char *month_num = sqlite3_column_text(stmt, 0);
        double sum_amount = sqlite3_column_double(stmt, 1);
        if (month_num) {
            int index = atoi((const char *)month_num) - 1;
            if (index >= 0 && index < 12) {
                income[index] = sum_amount;
            }
        }
    }
    sqlite3_finalize(stmt);

    // 4) We have arrays: expenses[0..11], income[0..11].
    //    Build a Chart.js config JSON:
    //    {
    //      "type":"bar",
    //      "data": {
    //        "labels": [...],
    //        "datasets": [ {...}, {...} ]
    //      },
    //      "options": {...}
    //    }

    // Build the labels array for all 12 months
    // e.g. ["January","February",...,"December"]
    char labels_json[1024] = "[";
    for (int i = 1; i <= 12; i++) {
        char monthStr[3];
        snprintf(monthStr, sizeof(monthStr), "%02d", i);
        // Append with commas
        char temp[128];
        snprintf(temp, sizeof(temp),
                 "\"%s\"%s",
                 month_number_to_name(monthStr),
                 (i < 12 ? "," : ""));
        strcat(labels_json, temp);
    }
    strcat(labels_json, "]");

    // Build the expenses data array: e.g. [120,90,100,...,150]
    char expenses_json[1024] = "[";
    for (int i = 0; i < 12; i++) {
        char temp[64];
        snprintf(temp, sizeof(temp),
                 "%.2f%s",
                 expenses[i],
                 (i < 11 ? "," : ""));
        strcat(expenses_json, temp);
    }
    strcat(expenses_json, "]");

    // Build the income data array
    char income_json[1024] = "[";
    for (int i = 0; i < 12; i++) {
        char temp[64];
        snprintf(temp, sizeof(temp),
                 "%.2f%s",
                 income[i],
                 (i < 11 ? "," : ""));
        strcat(income_json, temp);
    }
    strcat(income_json, "]");

    // Combine into a final JSON string for the bar chart config
    // Note: Adjust backgroundColor, etc. as needed
    char *chart_json = malloc(5000);
    if (!chart_json) {
        sqlite3_close(db);
        return strdup("{\"error\":\"Out of memory\"}");
    }

    snprintf(chart_json, 5000,
        "{"
          "\"type\":\"bar\","
          "\"data\":{"
            "\"labels\":%s,"
            "\"datasets\":["
              "{"
                "\"label\":\"Expenses\","
                "\"data\":%s,"
                "\"backgroundColor\":\"rgba(255, 99, 132, 0.2)\","
                "\"borderColor\":\"rgba(255, 99, 132, 1)\","
                "\"borderWidth\":1"
              "},"
              "{"
                "\"label\":\"Income\","
                "\"data\":%s,"
                "\"backgroundColor\":\"rgba(54, 162, 235, 0.2)\","
                "\"borderColor\":\"rgba(54, 162, 235, 1)\","
                "\"borderWidth\":1"
              "}"
            "]"
          "},"
          "\"options\":{"
            "\"scales\":{"
              "\"y\":{"
                "\"beginAtZero\":true"
              "}"
            "}"
          "}"
        "}",
        labels_json, expenses_json, income_json
    );

    sqlite3_close(db);
    return chart_json;
}

// -------------------------------------------------------------------
// handle_get_transactions_request
//
// This is the main function your server calls when the client wants
// the user’s transactions. It returns two sets of data in one JSON:
//
// {
//   "method1": [ ... array of transactions ... ],
//   "method2": {
//     "type": "bar",
//     "data": { ... },
//     "options": { ... }
//   }
// }
//
// - method1 = raw table data
// - method2 = bar chart config
// -------------------------------------------------------------------
char* handle_get_transactions_request(int user_id)
{
    // 1) Get the "raw table" list of transactions
    char *method1_data = get_transactions_raw_list(user_id);

    // 2) Get the "bar chart" config
    char *method2_data = get_barchart_config(user_id);

    // 3) Combine them into a single JSON object
    size_t needed = strlen(method1_data) + strlen(method2_data) + 128;
    char *final_json = (char*)malloc(needed);
    if (!final_json) {
        // Out of memory
        free(method1_data);
        free(method2_data);
        return strdup("{\"error\":\"Out of memory\"}");
    }

    snprintf(final_json, needed,
        "{"
          "\"method1\":%s,"
          "\"method2\":%s"
        "}",
        method1_data,
        method2_data
    );

    // 4) Free partial data
    free(method1_data);
    free(method2_data);

    // 5) Return the combined JSON
    return final_json;
}