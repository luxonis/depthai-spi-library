/*
 * spi_protocol.c
 *
 *  Created on: Mar 31, 2020
 *      Author: TheMarpe - Martin Peterlin
 *
 */

#include <spi_messaging.h>
#include <spi_protocol.h>

#include <string.h>
#include <math.h>
#include <assert.h>

#include <stdio.h>


uint8_t is_little_endian(){
    uint16_t i = 1;  
    char *c = (char*)&i;  
    return (*c) ? 1 : 0;
}

uint16_t read_uint16(uint8_t* currPtr){
    uint16_t result = 0;
    if(is_little_endian()){
        for(int i=0; i<sizeof(uint16_t); i++){
            result += *currPtr<<(i*8);
            currPtr++;
        }
    } else {
        for(int i=0; i<sizeof(uint16_t); i++){
            result += *currPtr<<((sizeof(uint16_t)-i-1)*8);
            currPtr++;
        }
    }
    return result;
}

uint32_t read_uint32(uint8_t* currPtr){
    uint32_t result = 0;
    if(is_little_endian()){
        for(int i=0; i<sizeof(uint32_t); i++){
            result += *currPtr<<(i*8);
            currPtr++;
        }
    } else {
        for(int i=0; i<sizeof(uint32_t); i++){
            result += *currPtr<<((sizeof(uint32_t)-i-1)*8);
            currPtr++;
        }
    }
    return result;
}



void spi_send_command(SpiProtocolPacket* spiPacket, spi_command command, uint8_t stream_name_len, char* stream_name){
    char sendPayload[SPI_PROTOCOL_PAYLOAD_SIZE] = {0};
    SpiCmdMessage spi_message;

    assert(stream_name_len <= MAX_STREAMNAME);

    spi_message.total_size = sizeof(spi_message.total_size) + sizeof(command) + sizeof(stream_name_len) + stream_name_len;
    spi_message.cmd = command;
    spi_message.stream_name_len = stream_name_len;
    printf("%d %s\n", strlen(stream_name), stream_name);
    strncpy(spi_message.stream_name, stream_name, stream_name_len);

    spi_protocol_write_packet(spiPacket, &spi_message, spi_message.total_size);
}

void spi_parse_command(SpiCmdMessage* parsed_message, uint8_t* data){
    uint8_t* currPtr = data;
    // read total_size - 2 bytes
    parsed_message->total_size = read_uint16(currPtr);
    currPtr = currPtr+2;

    // read cmd - 1 byte
    parsed_message->cmd = *currPtr;
    currPtr++;

    // read stream_name_len - 1 byte
    parsed_message->stream_name_len = *currPtr;
    currPtr++;

    // read streamName - up to 16 bytes
    memcpy(parsed_message->stream_name, (char*)currPtr, parsed_message->stream_name_len);
    // add a \0 after stream_name_len, just in case.
    parsed_message->stream_name[parsed_message->stream_name_len + 1] = "\0";
}

void spi_parse_get_size_resp(SpiGetSizeResp* parsedResp, uint8_t* data){
    parsedResp->size = read_uint32(data);
}

void spi_parse_pop_messages_resp(SpiPopMessagesResp* parsedResp, uint8_t* data){
    parsedResp->status = (uint8_t) data;
}

void spi_parse_get_streams_resp(SpiGetStreamsResp* parsedResp, uint8_t* data){
    uint8_t *currPtr = data;

    parsedResp->numStreams = (uint8_t) data[0];
    currPtr++;

    for(int i=0; i < parsedResp->numStreams; i++){
        strncpy(parsedResp->stream_names[i], (char *)currPtr, MAX_STREAMNAME);
        currPtr = currPtr + MAX_STREAMNAME;
    }
}

void spi_parse_get_message(SpiGetMessageResp* parsedResp, uint8_t* data, uint32_t total_size){
    
    parsedResp->total_size = total_size;

    
    // Pull two uint32_t off the end of the message. These are the metadata type and size.
    parsedResp->metadata_size = read_uint32(&data[total_size]-sizeof(uint32_t));
    parsedResp->metadata_type = read_uint32(&data[total_size]-2*sizeof(uint32_t));
    parsedResp->data_size = parsedResp->total_size - parsedResp->metadata_size - 2*sizeof(uint32_t);

    parsedResp->metadata = &data[parsedResp->data_size];
}
