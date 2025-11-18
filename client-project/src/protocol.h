/*
 * protocol.h
 *
 * Client header file
 * Definitions, constants and function prototypes for the client
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

// Shared application parameters
#define SERVER_PORT 27015  // Server port (change if needed)
#define BUFFER_SIZE 512    // Buffer size for messages

typedef struct {
    unsigned int status;  // Response status code
    char type;            // Echo of request type
    float value;          // Weather data value
} weather_response_t;

// Function prototypes
// Add here the signatures of the functions implemented by students
//TODO scrivere le funzioni

#endif /* PROTOCOL_H_ */
