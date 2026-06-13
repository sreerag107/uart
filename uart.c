#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define SOF_BYTE        0xAA
#define MAX_PAYLOAD     16

#define PARSER_OK       1
#define PARSER_BUSY     0
#define PARSER_CRC_ERR -1
#define PARSER_TIMEOUT -2

typedef enum
{
    STATE_WAIT_SOF = 0,
    STATE_CMD,
    STATE_LEN,
    STATE_PAYLOAD,
    STATE_CHECKSUM
} parser_state_t;

typedef struct
{
    parser_state_t state;

    uint8_t cmd;
    uint8_t len;
    uint8_t payload[MAX_PAYLOAD];

    uint8_t payload_index;
    uint8_t calculated_checksum;

    uint32_t timeout_ms;
    uint32_t last_timestamp;

} uart_parser_t;

static void parser_reset(uart_parser_t *parser)
{
    parser->state = STATE_WAIT_SOF;
    parser->cmd = 0;
    parser->len = 0;
    parser->payload_index = 0;
    parser->calculated_checksum = 0;
}

static void parser_init(uart_parser_t *parser, uint32_t timeout_ms)
{
    memset(parser, 0, sizeof(*parser));
    parser->timeout_ms = timeout_ms;
    parser->state = STATE_WAIT_SOF;
}

static void print_frame(const uart_parser_t *parser)
{
    uint8_t i;

    printf("FRAME OK CMD=0x%02X LEN=%u PAYLOAD=[",
           parser->cmd,
           parser->len);

    for(i = 0; i < parser->len; i++)
    {
        printf("%02X", parser->payload[i]);

        if(i < (parser->len - 1))
        {
            printf(" ");
        }
    }

    printf("]\n");
}

static int parser_feed_byte(uart_parser_t *parser,
                            uint8_t byte,
                            uint32_t timestamp_ms)
{
    if ((parser->timeout_ms > 0U) &&
        (parser->state != STATE_WAIT_SOF))
    {
        uint32_t gap = timestamp_ms - parser->last_timestamp;

        if (gap > parser->timeout_ms)
        {
            parser_reset(parser);
            return PARSER_TIMEOUT;
        }
    }

    parser->last_timestamp = timestamp_ms;

    switch(parser->state)
    {
        case STATE_WAIT_SOF:

            if(byte == SOF_BYTE)
            {
                parser->state = STATE_CMD;
            }
            break;

        case STATE_CMD:

            parser->cmd = byte;
            parser->calculated_checksum = byte;
            parser->state = STATE_LEN;
            break;

        case STATE_LEN:

            if(byte > MAX_PAYLOAD)
            {
                parser_reset(parser);
                break;
            }

            parser->len = byte;
            parser->calculated_checksum ^= byte;

            parser->payload_index = 0;

            if(parser->len == 0)
            {
                parser->state = STATE_CHECKSUM;
            }
            else
            {
                parser->state = STATE_PAYLOAD;
            }
            break;

        case STATE_PAYLOAD:

            parser->payload[parser->payload_index++] = byte;
            parser->calculated_checksum ^= byte;

            if(parser->payload_index >= parser->len)
            {
                parser->state = STATE_CHECKSUM;
            }
            break;

        case STATE_CHECKSUM:

            if(byte == parser->calculated_checksum)
            {
                parser->state = STATE_WAIT_SOF;
                return PARSER_OK;
            }
            else
            {
                parser_reset(parser);
                return PARSER_CRC_ERR;
            }

        default:
            parser_reset(parser);
            break;
    }

    return PARSER_BUSY;
}

static void feed_stream(uart_parser_t *parser,
                        const uint8_t *bytes,
                        const uint32_t *times,
                        uint32_t count)
{
    uint32_t i;

    for(i = 0; i < count; i++)
    {
        int result;

        result = parser_feed_byte(parser,
                                  bytes[i],
                                  times[i]);

        printf("t=%3ums byte=0x%02X -> ",
               times[i],
               bytes[i]);

        switch(result)
        {
            case PARSER_BUSY:
                printf("receiving...\n");
                break;

            case PARSER_OK:
                print_frame(parser);
                break;

            case PARSER_CRC_ERR:
                printf("CHECKSUM ERROR\n");
                break;

            case PARSER_TIMEOUT:
                printf("TIMEOUT -> parser reset\n");
                break;

            default:
                break;
        }
    }
}

int main(void)
{
    uart_parser_t parser;

    printf("\n===== TEST 1 =====\n");

    parser_init(&parser, 50);

    uint8_t test1[] =
    {
        0xAA,0x01,0x03,0x10,0x20,0x30,0x22
    };

    uint32_t time1[] =
    {
        0,5,10,15,20,25,30
    };

    feed_stream(&parser,
                test1,
                time1,
                sizeof(test1));

    printf("\n===== TEST 2 =====\n");

    parser_init(&parser, 50);

    uint8_t test2[] =
    {
        0xAA,0x01,0x03,0x10,
        0xAA,
        0xAA,0x05,0x01,0x7F,0x7B
    };

    uint32_t time2[] =
    {
        0,5,10,15,
        200,
        200,205,210,215,220
    };

    feed_stream(&parser,
                test2,
                time2,
                sizeof(test2));

    printf("\n===== TEST 3 =====\n");

    parser_init(&parser, 50);

    uint8_t test3[] =
    {
        0xAA,0x03,0x01,0x55,0x57,
        0xAA,0x04,0x02,0xAA,0xBB,0x17
    };

    uint32_t time3[] =
    {
        0,5,10,15,20,
        25,30,35,40,45,50
    };

    feed_stream(&parser,
                test3,
                time3,
                sizeof(test3));

    printf("\n===== TEST 4 =====\n");

    parser_init(&parser, 0);

    uint8_t test4[] =
    {
        0xAA,0x01,0x03,0x10,
        0xAA,
        0xAA,0x05,0x01,0x7F,0x7B
    };

    uint32_t time4[] =
    {
        0,5,10,15,
        200,
        200,205,210,215,220
    };

    feed_stream(&parser,
                test4,
                time4,
                sizeof(test4));

    return 0;
}
