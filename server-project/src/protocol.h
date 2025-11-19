/*
 * protocol.h
 *
 * Server header file
 * Definitions, constants and function prototypes for the server
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

// Shared application parameters
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 27015 // Server port (change if needed)
#define BUFFER_SIZE 512   // Buffer size for messages
#define QUEUE_SIZE 5      // Size of pending connections queue

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

/* Exported helpers (implemented in server) */
int parse_weather_request(const char *input, weather_request_t *req);
int format_weather_response(const weather_response_t *resp, char *buffer, size_t size);

/* Simple random value generators */
float get_temperature(void);
float get_humidity(void);    
float get_wind(void);       
float get_pressure(void);    

#endif /* PROTOCOL_H_ */
