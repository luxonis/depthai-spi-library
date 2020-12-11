#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "spi_api.hpp"




void SpiApi::debug_print_hex(uint8_t * data, int len){
    for(int i=0; i<len; i++){
        if(i%80==0){
            printf("\n");
        }
        printf("%02x", data[i]);
    }
    printf("\n");
}

void SpiApi::debug_print_char(char * data, int len){
    for(int i=0; i<len; i++){
        printf("%c", data[i]);
    }
    printf("\n");
}



SpiApi::SpiApi(){
    spi_proto_instance = (SpiProtocolInstance*) malloc(sizeof(SpiProtocolInstance));
    spi_send_packet = (SpiProtocolPacket*) malloc(sizeof(SpiProtocolPacket));
    spi_protocol_init(spi_proto_instance);
}

SpiApi::~SpiApi(){
    free(spi_proto_instance);
    free(spi_send_packet);
}

void SpiApi::set_send_spi_impl(uint8_t (*passed_send_spi)(char*)){
    send_spi_impl = passed_send_spi;
}

void SpiApi::set_recv_spi_impl(uint8_t (*passed_recv_spi)(char*)){
    recv_spi_impl = passed_recv_spi;
}

uint8_t SpiApi::generic_send_spi(char* spi_send_packet){
    return (*send_spi_impl)(spi_send_packet); 
}

uint8_t SpiApi::generic_recv_spi(char* recvbuf){
    return (*recv_spi_impl)(recvbuf);
}



uint8_t SpiApi::spi_get_size(SpiGetSizeResp *response, spi_command get_size_cmd, const char * stream_name){
    assert(isGetSizeCmd(get_size_cmd));

    uint8_t success = 0;
    debug_cmd_print("sending spi_get_size cmd.\n");
    spi_generate_command(spi_send_packet, get_size_cmd, strlen(stream_name)+1, stream_name);
    generic_send_spi((char*)spi_send_packet);

    debug_cmd_print("receive spi_get_size response from remote device...\n");
    char recvbuf[BUFF_MAX_SIZE] = {0};
    uint8_t recv_success = generic_recv_spi(recvbuf);

    if(recv_success){
        if(recvbuf[0]==START_BYTE_MAGIC){
            SpiProtocolPacket* spiRecvPacket = spi_protocol_parse(spi_proto_instance, (uint8_t*)recvbuf, sizeof(recvbuf));
            spi_parse_get_size_resp(response, spiRecvPacket->data);
            success = 1;
            
        }else if(recvbuf[0] != 0x00){
            printf("*************************************** got a half/non aa packet ************************************************\n");
            success = 0;
        }
    } else {
        printf("failed to recv packet\n");
        success = 0;
    }

    return success;
}

uint8_t SpiApi::spi_get_message(SpiGetMessageResp *response, spi_command get_mess_cmd, const char * stream_name, uint32_t size){
    assert(isGetMessageCmd(get_mess_cmd));

    uint8_t success = 0;
    debug_cmd_print("sending spi_get_message cmd.\n");
    spi_generate_command(spi_send_packet, get_mess_cmd, strlen(stream_name)+1, stream_name);
    generic_send_spi((char*)spi_send_packet);

    uint32_t total_recv = 0;
    int debug_skip = 0;
    while(total_recv < size){
        if(debug_skip%20 == 0){
            debug_cmd_print("receive spi_get_message response from remote device... %d/%d\n", total_recv, size);
        }
        debug_skip++;

        char recvbuf[BUFF_MAX_SIZE] = {0};
        uint8_t recv_success = generic_recv_spi(recvbuf);
        if(recv_success){
            if(recvbuf[0]==START_BYTE_MAGIC){
                SpiProtocolPacket* spiRecvPacket = spi_protocol_parse(spi_proto_instance, (uint8_t*)recvbuf, sizeof(recvbuf));
                uint32_t remaining_data = size-total_recv;
                if ( remaining_data < PAYLOAD_MAX_SIZE ){
                    memcpy(response->data+total_recv, spiRecvPacket->data, remaining_data);
                    total_recv += remaining_data;
                } else {
                    memcpy(response->data+total_recv, spiRecvPacket->data, PAYLOAD_MAX_SIZE);
                    total_recv += PAYLOAD_MAX_SIZE;
                }

            }else if(recvbuf[0] != 0x00){
                printf("*************************************** got a half/non aa packet ************************************************\n");
                break;
            }
        } else {
            printf("failed to recv packet\n");
            break;
        }
    }


    if(total_recv==size){
        spi_parse_get_message(response, size, get_mess_cmd);

        if(DEBUG_MESSAGE_CONTENTS){
            printf("data_size: %d\n", response->data_size);
            debug_print_hex((uint8_t*)response->data, response->data_size);
        }
        success = 1;
    } else {
        success = 0;
    }

    return success;
}

uint8_t SpiApi::spi_pop_messages(){
    SpiStatusResp response;
    uint8_t success = 0;

    debug_cmd_print("sending POP_MESSAGES cmd.\n");
    spi_generate_command(spi_send_packet, POP_MESSAGES, strlen(NOSTREAM)+1, NOSTREAM);
    generic_send_spi((char*)spi_send_packet);

    debug_cmd_print("receive POP_MESSAGES response from remote device...\n");
    char recvbuf[BUFF_MAX_SIZE] = {0};
    uint8_t recv_success = generic_recv_spi(recvbuf);

    if(recv_success){
        if(recvbuf[0]==START_BYTE_MAGIC){
            SpiProtocolPacket* spiRecvPacket = spi_protocol_parse(spi_proto_instance, (uint8_t*)recvbuf, sizeof(recvbuf));
            spi_status_resp(&response, spiRecvPacket->data);
            if(response.status == SPI_MSG_SUCCESS_RESP){
                success = 1;
            }
            
        }else if(recvbuf[0] != 0x00){
            printf("*************************************** got a half/non aa packet ************************************************\n");
            success = 0;
        }
    } else {
        printf("failed to recv packet\n");
        success = 0;
    }

    return success;
}

uint8_t SpiApi::spi_get_streams(SpiGetStreamsResp *response){
    uint8_t success = 0;
    debug_cmd_print("sending GET_STREAMS cmd.\n");
    spi_generate_command(spi_send_packet, GET_STREAMS, 1, NOSTREAM);
    generic_send_spi((char*)spi_send_packet);

    debug_cmd_print("receive GET_STREAMS response from remote device...\n");
    char recvbuf[BUFF_MAX_SIZE] = {0};
    uint8_t recv_success = generic_recv_spi(recvbuf);

    if(recv_success){
        if(recvbuf[0]==START_BYTE_MAGIC){
            SpiProtocolPacket* spiRecvPacket = spi_protocol_parse(spi_proto_instance, (uint8_t*)recvbuf, sizeof(recvbuf));
            spi_parse_get_streams_resp(response, spiRecvPacket->data);
            success = 1;
            
        }else if(recvbuf[0] != 0x00){
            printf("*************************************** got a half/non aa packet ************************************************\n");
            success = 0;
        }
    } else {
        printf("failed to recv packet\n");
        success = 0;
    }

    return success;
}

uint8_t SpiApi::spi_pop_message(const char * stream_name){
    uint8_t success = 0;
    SpiStatusResp response;

    debug_cmd_print("sending POP_MESSAGE cmd.\n");
    spi_generate_command(spi_send_packet, POP_MESSAGE, strlen(stream_name)+1, stream_name);
    generic_send_spi((char*)spi_send_packet);

    debug_cmd_print("receive POP_MESSAGE response from remote device...\n");
    char recvbuf[BUFF_MAX_SIZE] = {0};
    uint8_t recv_success = generic_recv_spi(recvbuf);

    if(recv_success){
        if(recvbuf[0]==START_BYTE_MAGIC){
            SpiProtocolPacket* spiRecvPacket = spi_protocol_parse(spi_proto_instance, (uint8_t*)recvbuf, sizeof(recvbuf));
            spi_status_resp(&response, spiRecvPacket->data);
            if(response.status == SPI_MSG_SUCCESS_RESP){
                success = 1;
            }
            
        }else if(recvbuf[0] != 0x00){
            printf("*************************************** got a half/non aa packet ************************************************\n");
            success = 0;
        }
    } else {
        printf("failed to recv packet\n");
        success = 0;
    }

    return success;
}




uint8_t SpiApi::req_data(SpiGetMessageResp *get_message_resp, const char* stream_name){
    uint8_t req_success = 0;

    // do a get_size before trying to retreive message.
    SpiGetSizeResp get_size_resp;
    req_success = spi_get_size(&get_size_resp, GET_SIZE, stream_name);
    debug_cmd_print("response: %d\n", get_size_resp.size);

    // get message (assuming we got size)
    if(req_success){
        get_message_resp->data = (uint8_t*) malloc(get_size_resp.size);
        if(!get_message_resp->data){
            printf("failed to allocate %d bytes", get_size_resp.size);
        }

        req_success = spi_get_message(get_message_resp, GET_MESSAGE, stream_name, get_size_resp.size);
    }

    return req_success;
}

uint8_t SpiApi::req_metadata(SpiGetMessageResp *get_message_resp, const char* stream_name){
    uint8_t req_success = 0;

    // do a get_size before trying to retreive message.
    SpiGetSizeResp get_size_resp;
    req_success = spi_get_size(&get_size_resp, GET_METASIZE, stream_name);
    debug_cmd_print("response: %d\n", get_size_resp.size);

    // get message (assuming we got size)
    if(req_success){
        get_message_resp->data = (uint8_t*) malloc(get_size_resp.size);
        if(!get_message_resp->data){
            printf("failed to allocate %d bytes", get_size_resp.size);
        }

        req_success = spi_get_message(get_message_resp, GET_METADATA, stream_name, get_size_resp.size);
    }

    return req_success;
}

/*
struct full_message{
    SpiGetMessageResp* data_resp;
    SpiGetMessageResp* meta_resp;
}

req_full_msg();
*/
