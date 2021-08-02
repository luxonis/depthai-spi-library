#ifndef _DEPTHAI_SPI_MESSAGING_H_
#define _DEPTHAI_SPI_MESSAGING_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define DEPTHAI_SPI_DEFAULT_MESSAGE_SIZE 64
#define DEPTHAI_SPI_MAX_STREAM_NAME 16

// Commands
typedef enum : uint32_t {
    DEPTHAI_SPI_GET_SIZE,
    DEPTHAI_SPI_GET_DATA,
    DEPTHAI_SPI_GET_METADATA,
    DEPTHAI_SPI_POP_MESSAGE,
    DEPTHAI_SPI_GET_NUM_STREAM,
    DEPTHAI_SPI_GET_STREAM,
    DEPTHAI_SPI_SEND_DATA,
} depthai_spi_command;

// Raw message structure - fixed to a certain size
typedef struct {
    uint8_t message[DEPTHAI_SPI_DEFAULT_MESSAGE_SIZE];
} depthai_spi_message;

// Base Command structure
typedef struct {
    uint32_t checksum;
    depthai_spi_command command;
} depthai_spi_base_message;

// GetSize response structure
typedef struct {
    depthai_spi_base_message base;
    char stream_name[DEPTHAI_SPI_MAX_STREAM_NAME];
    uint32_t data_size;
    uint32_t metadata_size;
} depthai_spi_get_size_response;


// GetDataRequest command
typedef struct {
    depthai_spi_base_message base;
    char stream_name[DEPTHAI_SPI_MAX_STREAM_NAME];
    uint32_t offset;
    uint32_t size;
} depthai_spi_get_data_request;

// GetMetadataRequest command
typedef struct {
    depthai_spi_base_message base;
    char stream_name[DEPTHAI_SPI_MAX_STREAM_NAME];
    uint32_t offset;
    uint32_t size;
} depthai_spi_get_metadata_request;


// GetNumStream response
typedef struct {
    depthai_spi_base_message base;
    uint32_t num;
} depthai_spi_get_num_streams_response;


// GetStream request
typedef struct {
    depthai_spi_base_message base;
    uint32_t num;
    uint32_t offset;
} depthai_spi_get_stream_request;

// GetStream response
typedef struct {
    depthai_spi_base_message base;
    uint32_t num;
    uint32_t offset;
} depthai_spi_get_stream_response;

// Stream entry
typedef struct {
    char stream_name[DEPTHAI_SPI_MAX_STREAM_NAME];
} depthai_spi_stream_entry;

// Checksum
static inline uint32_t depthai_spi_initial_checksum() {
    return 5381;
}
uint32_t depthai_spi_compute_checksum_prev(const void* buffer, uint32_t size, uint32_t prev_checksum);
uint32_t depthai_spi_compute_checksum(const void* buffer, uint32_t size);


/// Finalizes any given message (calculates checksum)
void depthai_spi_finalize_message(void* message, depthai_spi_command command);
/// Creates a (base) message with given command
depthai_spi_message depthai_spi_create_message(depthai_spi_command command);

#ifdef __cplusplus
}
#endif


#endif // _DEPTHAI_SPI_MESSAGING_H_