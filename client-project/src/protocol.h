/*
 * protocol.h
 *
 * Client header file
 * Definitions, constants and function prototypes for the client
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

// Shared application parameters
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 27015 // Server port (change if needed)
#define BUFFER_SIZE 512   // Buffer size for messages

typedef struct
{
    char type;     // Weather data type: 't', 'h', 'w', 'p'
    char city[64]; // City name (null-terminated string)
} weather_request_t;

typedef struct
{
    unsigned int status; // Response status code
    char type;           // Echo of request type
    float value;         // Weather data value
} weather_response_t;

/* Helper function prototypes (implemented in the examples) */
int parse_weather_response(const char *input, weather_response_t *resp);
void parse_weather_request(const char *input, weather_request_t *req);

#endif /* PROTOCOL_H_ */
