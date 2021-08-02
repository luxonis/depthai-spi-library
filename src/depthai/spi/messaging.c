#include <depthai/spi/messaging.h>

// std
#include <string.h>
#include <math.h>
#include <assert.h>
#include <stdio.h>

// Checksum calculation
uint32_t depthai_spi_compute_checksum_prev(const void* buffer, uint32_t size, uint32_t prev_checksum) {
    uint32_t checksum = prev_checksum;
    uint8_t* p = (uint8_t*)buffer;

    for(unsigned int i = 0; i < size; i++) {
        checksum = ((checksum << 5) + checksum) + p[i]; /* hash * 33 + p[i] */
    }

    return checksum;
}

uint32_t depthai_spi_compute_checksum(const void* buffer, uint32_t size) {
    return depthai_spi_compute_checksum_prev(buffer, size, depthai_spi_initial_checksum());
}

// Commands

// Takes 'message' pointer and assumes its depthai_spi_base_message, and applies correct checksum for it
void depthai_spi_finalize_message(void* message, depthai_spi_command command){
    depthai_spi_base_message* msg = (depthai_spi_base_message*) message;
    msg->command = command;
    cmd->checksum = depthai_spi_compute_checksum(&cmd->command, DEPTHAI_SPI_DEFAULT_MESSAGE_SIZE - sizeof(msg->checksum));
}

// Creates a message
depthai_spi_message depthai_spi_create_message(depthai_spi_command command){
    // Create a clear msg
    depthai_spi_message msg = {0};
    // finalize with specified command
    depthai_spi_finalize_message(msg, command);
    return msg;
}



uint8_t is_little_endian(){
    uint16_t i = 1;
    char *c = (char*)&i;
    return (*c) ? 1 : 0;
}

uint16_t read_uint16(uint8_t* currPtr){
    uint16_t result = 0;
    if(is_little_endian()){
        for(size_t i=0; i<sizeof(uint16_t); i++){
            result += *currPtr<<(i*8);
            currPtr++;
        }
    } else {
        for(size_t i=0; i<sizeof(uint16_t); i++){
            result += *currPtr<<((sizeof(uint16_t)-i-1)*8);
            currPtr++;
        }
    }
    return result;
}

uint32_t read_uint32(uint8_t* currPtr){
    uint32_t result = 0;
    if(is_little_endian()){
        for(size_t i=0; i<sizeof(uint32_t); i++){
            result += *currPtr<<(i*8);
            currPtr++;
        }
    } else {
        for(size_t i=0; i<sizeof(uint32_t); i++){
            result += *currPtr<<((sizeof(uint32_t)-i-1)*8);
            currPtr++;
        }
    }
    return result;
}


uint8_t isGetSizeCmd(spi_command cmd){
    uint8_t result = 0;
    for(size_t i=0; i < sizeof(GET_SIZE_CMDS) / sizeof(GET_SIZE_CMDS[0]); i++){
        if(cmd == GET_SIZE_CMDS[i]){
            result = 1;
        }
    }
    return result;
}

uint8_t isGetMessageCmd(spi_command cmd){
    uint8_t result = 0;
    for(size_t i=0; i < sizeof(GET_MESS_CMDS) / sizeof(GET_MESS_CMDS[0]); i++){
        if(cmd == GET_MESS_CMDS[i]){
            result = 1;
        }
    }
    return result;
}


void spi_generate_command(SpiProtocolPacket* spiPacket, spi_command command, uint8_t stream_name_len, const char* stream_name){
    SpiCmdMessage spi_message;

    assert(stream_name_len <= MAX_STREAMNAME);

    spi_message.total_size = sizeof(spi_message.total_size) + sizeof(command) + sizeof(stream_name_len) + sizeof(spi_message.extra_offset) + sizeof(spi_message.extra_size) + stream_name_len;
    spi_message.cmd = command;
    spi_message.stream_name_len = stream_name_len;
    spi_message.extra_offset = 0;
    spi_message.extra_size = 0;
    strncpy(spi_message.stream_name, stream_name, stream_name_len);

    spi_protocol_write_packet(spiPacket, (uint8_t*) &spi_message, spi_message.total_size);
}

void spi_generate_command_partial(SpiProtocolPacket* spiPacket, spi_command command, uint8_t stream_name_len, const char* stream_name, uint32_t offset, uint32_t offset_size){
    SpiCmdMessage spi_message;

    assert(stream_name_len <= MAX_STREAMNAME);

    spi_message.total_size = sizeof(spi_message.total_size) + sizeof(command) + sizeof(stream_name_len) + sizeof(spi_message.extra_offset) + sizeof(spi_message.extra_size) + stream_name_len;
    spi_message.cmd = command;
    spi_message.stream_name_len = stream_name_len;
    spi_message.extra_offset = offset;
    spi_message.extra_size = offset_size;
    strncpy(spi_message.stream_name, stream_name, stream_name_len);

    spi_protocol_write_packet(spiPacket, (uint8_t*) &spi_message, spi_message.total_size);
}

void spi_generate_command_send(SpiProtocolPacket* spiPacket, spi_command command, uint8_t stream_name_len, const char* stream_name, uint32_t metadata_size, uint32_t send_data_size){
    SpiCmdMessage spi_message;

    assert(stream_name_len <= MAX_STREAMNAME);

    spi_message.total_size = sizeof(spi_message.total_size) + sizeof(command) + sizeof(stream_name_len) + sizeof(spi_message.extra_offset) + sizeof(spi_message.extra_size) + stream_name_len;
    spi_message.cmd = command;
    spi_message.stream_name_len = stream_name_len;
    spi_message.extra_offset = 0;
    spi_message.extra_size = send_data_size;
    spi_message.metadata_size = metadata_size;
    strncpy(spi_message.stream_name, stream_name, stream_name_len);

    spi_protocol_write_packet(spiPacket, (uint8_t*) &spi_message, spi_message.total_size);
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

    // read extra_offset - 4 bytes
    parsed_message->extra_offset = read_uint32(currPtr);
    currPtr = currPtr+4;

    // read extra_size - 4 bytes
    parsed_message->extra_size = read_uint32(currPtr);
    currPtr = currPtr+4;

    // read metadata_size - 4 bytes
    parsed_message->metadata_size = read_uint32(currPtr);
    currPtr = currPtr+4;

    // read streamName - up to 16 bytes
    assert(parsed_message->stream_name_len <= MAX_STREAMNAME);
    memcpy(parsed_message->stream_name, (char*)currPtr, parsed_message->stream_name_len);
}

void spi_parse_get_size_resp(SpiGetSizeResp* parsedResp, uint8_t* data){
    parsedResp->size = read_uint32(data);
}

void spi_status_resp(SpiStatusResp* parsedResp, uint8_t* data){
    parsedResp->status = (uint8_t) *data;
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

void spi_parse_get_message(SpiGetMessageResp* parsedResp, uint32_t size, spi_command get_mess_cmd){
    switch(get_mess_cmd){
        case GET_MESSAGE: {
            parsedResp->data_type = 0;      //currently unused.
            parsedResp->data_size = size;
        } break;
        case GET_METADATA: {
            parsedResp->data_type = read_uint32(&parsedResp->data[size]-2*sizeof(uint32_t));
            parsedResp->data_size = read_uint32(&parsedResp->data[size]-sizeof(uint32_t));      // use the size at the end of this message, it chops off the extra bytes for type and message size.
        } break;
        case GET_MESSAGE_PART: {
            parsedResp->data_type = 0;      //currently unused.
            parsedResp->data_size = size;
        } break;
        default: {
            printf("Warning: Unsupported spi_command passed to %s.", __func__);
        }break;
    }
}
