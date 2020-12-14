#ifndef SHARED_SPI_API_H
#define SHARED_SPI_API_H

#include <assert.h>

#include "spi_messaging.h"
#include "spi_protocol.h"

#include "SpiPacketParser.hpp"
#include "depthai-shared/datatype/RawImgDetections.hpp"

#define DEBUG_CMD 0
#define debug_cmd_print(...) \
    do { if (DEBUG_CMD) fprintf(stderr, __VA_ARGS__); } while (0)

#define DEBUG_MESSAGE_CONTENTS 0


static const char* NOSTREAM = "";

typedef struct {
    SpiGetMessageResp raw_data_resp;
    SpiGetMessageResp raw_meta_resp;
    dai::DatatypeEnum metatype;
    void* parsed_meta;
} FullMessage;

class SpiApi {
    private:
        uint8_t (*send_spi_impl)(char* spi_send_packet);
        uint8_t (*recv_spi_impl)(char* recvbuf);

        void (*chunk_message_cb)(char* curr_packet, uint32_t chunk_size, uint32_t message_size);

        SpiProtocolInstance* spi_proto_instance;
        SpiProtocolPacket* spi_send_packet;

    public:
        SpiApi();
        ~SpiApi();

        // debug stuff
        void debug_print_hex(uint8_t * data, int len);
        void debug_print_char(char * data, int len);

        // refs to callbacks
        void set_send_spi_impl(uint8_t (*passed_send_spi)(char*));
        void set_recv_spi_impl(uint8_t (*passed_recv_spi)(char*));
        uint8_t generic_send_spi(char* spi_send_packet);
        uint8_t generic_recv_spi(char* recvbuf);

        // base SPI command methods
        uint8_t spi_get_size(SpiGetSizeResp *response, spi_command get_size_cmd, const char * stream_name);
        uint8_t spi_get_message(SpiGetMessageResp *response, spi_command get_mess_cmd, const char * stream_name, uint32_t size);
        uint8_t spi_pop_messages();
        uint8_t spi_get_streams(SpiGetStreamsResp *response);
        uint8_t spi_pop_message(const char * stream_name);
        
        uint8_t req_data(SpiGetMessageResp *get_message_resp, const char* stream_name);
        uint8_t req_metadata(SpiGetMessageResp *get_message_resp, const char* stream_name);

        uint8_t req_full_msg(FullMessage* received_msg, const char* stream_name);
        void free_full_msg(FullMessage* received_msg);


        void parse_metadata(SpiGetMessageResp *raw_meta_resp);
        void chunk_message(const char* stream_name);
        void set_chunk_packet_cb(void (*passed_chunk_message_cb)(char*, uint32_t, uint32_t));
};



#endif
