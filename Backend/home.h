#ifndef HOME_H
#define HOME_H

/**
 * Handle the `/home` request.
 * 
 * @param request The raw HTTP request string.
 * @return Dynamically allocated string containing the HTTP response.
 *         The caller must free the returned string.
 */
char *handle_home_request(const char *request);

#endif
