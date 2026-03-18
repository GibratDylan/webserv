# WebServ - HTTP Server

*This project has been created as part of the 42 curriculum by dgibrat, sskobyak.*

## Description

WebServ is a fully-functional HTTP web server written in C++98 that demonstrates core networking and web server concepts. It is a non-blocking, high-performance server capable of handling multiple concurrent connections using I/O multiplexing (poll/epoll).

### Key Features

- **Multiple Server Instances**: Support for running multiple servers on different ports with independent configurations
- **HTTP Methods**: Support for GET, POST, and DELETE request methods with proper handling
- **CGI Support**: Execute scripts and programs (Python, PHP, Node.js, Bash, etc.) as CGI processes
- **File Serving**: Efficient static file serving with directory listing (autoindex) capabilities
- **Location-Based Routing**: Configure different behaviors for different URL paths within a server
- **Session Management**: Track user sessions with configurable timeouts
- **Error Handling**: Custom error pages for various HTTP error codes (400, 403, 404, 405, 413, 500, 501, 504, etc.)
- **Request Parsing**: Robust HTTP request parsing with support for headers and body content
- **Chunked Encoding**: Support for chunked transfer encoding
- **Redirects**: Location-based HTTP redirects (301 responses)
- **File Uploads**: Handle file uploads to specified directories with configurable size limits
- **Signal Handling**: Graceful shutdown handling via signal system
- **Logging**: Comprehensive logging system with configurable verbosity levels
- **Caching**: File and response caching for performance optimization

## Architecture

The project follows a modular architecture organized into several key components:

- **Server**: Core server logic, connection management, and event loop
- **Config**: Configuration file parsing and server/location configuration management
- **HTTP**: HTTP request/response handling and status code management
- **Network**: I/O multiplexing and socket management
- **CGI**: CGI handler for executing external scripts
- **Utility**: Helper utilities including logger, file system operations, signal handling, and caching

## Instructions

### Requirements

- C++98 compatible compiler (g++, clang++)
- Standard POSIX environment (Linux)
- Python3, PHP-CGI, Node.js (optional, for CGI demonstrations)

### Compilation

Navigate to the project root directory and run:

```bash
make
```

This will compile the webserver and produce a `webserv` executable.

To clean the build artifacts:

```bash
make clean
```

To remove all generated files:

```bash
make fclean
```

To rebuild from scratch:

```bash
make re
```

### Installation

No installation is required. After compilation, the `webserv` executable can be run directly from the project directory.

### Execution

Run the server with the default configuration:

```bash
./webserv
```

Run the server with a custom configuration file:

```bash
./webserv config/your_config.conf
```

The server will start and listen on the ports specified in the configuration file. By default, it listens on ports 8080, 8888, and 9090 (as configured in `config/default.conf`).

### Configuration

The server is configured via `.conf` files (located in the `config/` directory). A configuration file specifies:

- Global settings (root directory, default index files, client body size limits, etc.)
- Multiple servers with individual settings
- Locations within servers with different behaviors

Example configuration directives:

```
root ./www;
index index.html;
server {
    listen 8080;
    root ./www/8080;
    allow_methods GET POST DELETE;
    client_max_body_size 10000000;
    
    location /files/ {
        root ./www/8080/files;
        autoindex on;
    }
    
    location /upload {
        upload_store ./www/8080/uploads;
    }
    
    location /api/ {
        cgi /usr/bin/python3;
    }
}
```

Refer to `config/valid_basic.conf`, `config/valid_complex.conf`, and `config/default.conf` for additional examples.

### Stopping the Server

Press `Ctrl+C` to gracefully shutdown the server.

## Testing

The project includes testing utilities and example scripts:

- `tests/chunked.py`: Script to test chunked transfer encoding
- `tests/stress/get_1000.sh`: Stress test with 1000 GET requests
- `tests/stress/post_1000.sh`: Stress test with 1000 POST requests
- `tests/stress/upload_1000.sh`: Stress test for file uploads
- `tests/stress/siege.sh`: Siege load test (optional; skipped if `siege` isn't installed)
- `www/`: Directory containing test web content and CGI scripts

Run tests using the included test scripts:

```bash
./tests/stress/get_1000.sh   # Test concurrent GET requests
./tests/stress/post_1000.sh  # Test concurrent POST requests
```

## Resources

### HTTP and Web Server References

- [RFC 7230 - HTTP/1.1 Message Syntax and Routing](https://tools.ietf.org/html/rfc7230)
- [RFC 7231 - HTTP/1.1 Semantics and Content](https://tools.ietf.org/html/rfc7231)
- [RFC 3875 - CGI Specification](https://tools.ietf.org/html/rfc3875)
- [MDN Web Docs - HTTP](https://developer.mozilla.org/en-US/docs/Web/HTTP)
- [Nginx Configuration Documentation](https://nginx.org/en/docs/)

### Networking and Socket Programming

- [POSIX Socket Programming](https://www.man7.org/linux/man-pages/man7/socket.7.html)
- [poll() System Call Reference](https://www.man7.org/linux/man-pages/man2/poll.2.html)
- [epoll() System Call Reference](https://www.man7.org/linux/man-pages/man7/epoll.7.html)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)

### C++ References

- [cppreference.com](https://en.cppreference.com/) - C++ Standard Library Reference
- [C++98 Standard Library](https://en.cppreference.com/w/cpp/98)

### Related Topics

- [HTTP Status Codes - MDN](https://developer.mozilla.org/en-US/docs/Web/HTTP/Status)
- [URL Encoding and RFC 3986](https://tools.ietf.org/html/rfc3986)
- [Chunked Transfer Encoding - RFC 7230](https://tools.ietf.org/html/rfc7230#section-4.1)

## AI Usage

AI assistance was utilized for the following tasks and components:

- **Code Review and Refactoring**: AI was used to review code structure, identify potential improvements, and refactor components for better maintainability and performance
- **Debugging and Error Handling**: AI assisted in identifying edge cases, debugging complex logic in HTTP parsing and request handling, and improving exception handling
- **Documentation**: AI helped generate this README and provided guidance on structuring project documentation
- **Algorithm Optimization**: AI provided insights on optimizing I/O multiplexing patterns and connection management
- **Code Quality**: AI suggested improvements for following C++98 best practices, reducing code duplication, and improving code clarity

## Project Structure

```
webserv/
├── src/              # Source files
│   ├── main.cpp
│   ├── server/       # Server and HTTP handling
│   ├── config/       # Configuration parsing
│   ├── network/      # Network I/O multiplexing
│   ├── cgi/          # CGI handler
│   ├── http/         # HTTP status codes
│   └── utility/      # Utilities (logger, filesystem, caching)
├── include/          # Header files (mirrors src structure)
├── config/           # Configuration files for testing
├── www/              # Web root directories for different servers
├── tests/            # Testing scripts and utilities
└── Makefile          # Build configuration
```

## Notes

- The server is implemented using C++98 and does not use C++11 or later features
- The project uses only standard library features compatible with C++98
- The server is single-threaded and uses I/O multiplexing for concurrency
- All memory management follows RAII principles with proper cleanup

## Troubleshooting

### Server fails to start with "Address already in use"
The port is still in use by another process. Wait a few seconds after shutting down, or change the port in your configuration file.

### CGI scripts return 502 Bad Gateway
Ensure the CGI interpreter is installed on your system (e.g., `python3`, `php-cgi`) and the path is correct in your config file.

### Files not being served correctly
Check that the `root` directive in your configuration points to the correct directory relative to where you run `./webserv`.

---

For more information or issues, refer to the configuration examples in the `config/` directory and the test cases in `tests/`.
