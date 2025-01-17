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
static const char* month_number_to_name(const char* monthNum) {
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
// Example:
//   [
//     {
//       "id": 6, "trans_type": "expense", "amount": 200.00,
//       "date": "2025-01-17", "category": "Transport"
//     },
//     ...
//   ]
// -------------------------------------------------------------------
static char* get_transactions_raw_list(int user_id) {
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
            id, trans_type, amount, date, category
        );

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
// METHOD 2 DATA: Return a Chart.js "bar" config object
// (suitable for a bar chart in your frontend).
//
// Example:
//   {
//     "type": "bar",
//     "data": {
//       "labels": ["January","February","March","April","May","June"],
//       "datasets": [
//         {
//           "label": "Expenses",
//           "data": [120, 90, 100, 75, 200, 150],
//           ...
//         },
//         {
//           "label": "Income",
//           "data": [300, 250, 200, 280, 350, 400],
//           ...
//         }
//       ]
//     },
//     "options": {
//       "scales": { "y": { "beginAtZero": true } }
//     }
//   }
// -------------------------------------------------------------------
static char* get_barchart_config(int user_id) {
    /*
      Here you could do a real query to group by month and produce
      dynamic "labels" and "data" arrays. For simplicity, let's just
      return a static example.
    */
    const char *json_example =
        "{"
          "\"type\":\"bar\","
          "\"data\":{"
            "\"labels\":[\"January\",\"February\",\"March\",\"April\",\"May\",\"June\"],"
            "\"datasets\":["
              "{"
                "\"label\":\"Expenses\","
                "\"data\":[120,90,100,75,200,150],"
                "\"backgroundColor\":\"rgba(255, 99, 132, 0.2)\","
                "\"borderColor\":\"rgba(255, 99, 132, 1)\","
                "\"borderWidth\":1"
              "},"
              "{"
                "\"label\":\"Income\","
                "\"data\":[300,250,200,280,350,400],"
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
        "}";

    // Return a copy on the heap
    return strdup(json_example);
}

// -------------------------------------------------------------------
// handle_get_transactions_request
//
// As requested: “Update the code to as default return two types of data”
//   - method1: “normal table” data (raw array of transactions)
//   - method2: a “barchart” config for Chart.js
//
// We'll combine them into a single JSON object like:
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
// So your front end can do:
//   const fullData = JSON.parse(...);
//   // fullData.method1 -> table array
//   // fullData.method2 -> bar chart config
// -------------------------------------------------------------------
char* handle_get_transactions_request(int user_id)
{
    // 1) Get Method 1 data (raw list for a table)
    char *method1_data = get_transactions_raw_list(user_id);

    // 2) Get Method 2 data (bar chart config)
    char *method2_data = get_barchart_config(user_id);

    // 3) Combine into a single JSON object:
    //    { "method1": <method1_data>, "method2": <method2_data> }
    //
    // Because method1_data is a JSON array (e.g. `[ ... ]`),
    // and method2_data is a JSON object (e.g. `{ "type":"bar", ... }`),
    // we can embed them directly.

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
    printf("method1_data: %s\n", method1_data);
    printf("method2_data: %s\n", method2_data);
    // 4) Free the partial JSON strings
    free(method1_data);
    free(method2_data);

    // 5) Return the combined result
    return final_json;
}
