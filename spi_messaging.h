#ifndef SHARED_SPI_MESSAGING_H
#define SHARED_SPI_MESSAGING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <spi_protocol.h>

#define MAX_STREAMNAME 16
#define MAX_STREAMS 12

typedef enum{
    GET_MESSAGE,
    GET_SIZE,
    POP_MESSAGES,
    GET_STREAMS
} spi_command;

typedef struct {
    uint16_t total_size;
    uint8_t cmd;
    uint8_t stream_name_len;
    char stream_name[MAX_STREAMNAME];
} SpiCmdMessage;

typedef struct {
    uint32_t size;
} SpiGetSizeResp; 

typedef struct {
    uint32_t total_size;
    uint32_t data_size;
    uint32_t metadata_size;
    uint32_t metadata_type;
    uint8_t *data;
    uint8_t *metadata;
} SpiGetMessageResp;

typedef struct {
    uint8_t status;
} SpiPopMessagesResp;

typedef struct {
    uint8_t numStreams;
    char stream_names[MAX_STREAMS][MAX_STREAMNAME]; 
} SpiGetStreamsResp;

void spi_generate_command(SpiProtocolPacket* spiPacket, spi_command command, uint8_t streamNameLen, char* streamName);
void spi_parse_command(SpiCmdMessage* message, uint8_t* data);

void spi_parse_get_size_resp(SpiGetSizeResp* parsedResp, uint8_t* data);
void spi_parse_pop_messages_resp(SpiPopMessagesResp* parsedResp, uint8_t* data);
void spi_parse_get_streams_resp(SpiGetStreamsResp* parsedResp, uint8_t* data);
void spi_parse_get_message(SpiGetMessageResp* parsedResp, uint8_t* data, uint32_t total_size);

#ifdef __cplusplus
}
#endif


#endif
