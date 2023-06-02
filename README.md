# HttpServer

This is a simple server implementing Http without 3-party libs

# Features supported

This server supports serving GET and POST request of a certain types
If request is correct the response is with status code 200 OK
If GET request contains syntax error (HOSL instead of HOST) the response is with status code 400 Bad Request
If resource does not equal /index.html the response is with status code 404 Not Found
