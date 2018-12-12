#ifndef _STATUS_H
#define _STATUS_H

typedef struct {
    char *ext;
    char *mime_type;
} extn;

extn extensions[] = {
    {"htm", "text/html"},
    {"html", "text/html"},
    {"css", "text/css"},
    {"h", "text/x-h"},
    {"hh", "text/x-h"},
    {"c", "text/x-c"},
    {"cc", "text/x-c"},
    {"json", "application/json"},
    {0, 0}
};

enum {
    OK = 0,
    BAD_REQUEST,
    NOT_FOUND,
    METHOD_NOT_ALLOWED,
    UNSUPPORT_MEDIA_TYPE
};

const int status_code[] = {
    200, /* OK */
    400, /* Bad Request */	// doesn't start with "/"
    404, /* Not Found */	// no file or dir
    405, /* Method Not Allowed */ 	// Support only "GET"
    415, /* Unsupported Media Type */ // Unsupported type
};

#endif
