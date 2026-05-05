/*!
 * \file test_bms-manager.c
 * \date 2024-06-23
 * \author Riccardo Segala [riccardo.segala@icloud.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 * 
 * \brief Test functions for the bms-manager module
 */

#include "unity.h"
#include "bms-manager-api.h"
#include "identity-api.h"
#include "cellboard-def.h"
#include "ltc6811-1-api.h"
#include "fff.h"
DEFINE_FFF_GLOBALS;

static void append_pec(uint8_t *frame) {
    uint16_t pec = prv_ltc6811_1_pec15(frame, 6);
    frame[6] = (pec >> 8) & 0xFF;
    frame[7] = pec & 0xFF;
}

#define CELLBOARD_ID CELLBOARD_ID_1

uint8_t captured_data[LTC6811_1_WRITE_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)] = { 0 };

uint8_t captured_out_data[LTC6811_1_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)] = { 0 };

FAKE_VALUE_FUNC(enum BmsManagerReturnCode, send, uint8_t *const, const size_t);
FAKE_VALUE_FUNC(enum BmsManagerReturnCode, send_receive, uint8_t *, uint8_t *, const size_t, const size_t);

enum BmsManagerReturnCode prv_bms_manager_send(uint8_t *const data, const size_t size);
extern struct BmsManagerHandler bms_handler;

void test_bms_manager_init_correct_values(void) {

    struct BmsManagerHandler expected_handler = {
        .send = send,
        .send_receive = send_receive,
        .ltc_handler = { .count = CELLBOARD_SEGMENT_LTC_COUNT }
    };

    for (size_t i = 0U; i < CELLBOARD_SEGMENT_LTC_COUNT; ++i) {
        expected_handler.requested_config[i].GPIO = 0b11111;
        expected_handler.requested_config[i].REFON = 1U;
    }

    enum BmsManagerReturnCode code = bms_manager_init(send, send_receive);
    TEST_ASSERT_EQUAL_MESSAGE(BMS_MANAGER_RC_OK, code, "bms_manager_init should return BMS_MANAGER_RC_OK if the handler is initialized correctly");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(&expected_handler, &bms_handler, sizeof(expected_handler), "bms_manager_init should initialize the handler structure with the correct values");
}

void test_bms_manager_init_null_send_receive(void) {

    enum BmsManagerReturnCode code = bms_manager_init(send, NULL);

    TEST_ASSERT_EQUAL_MESSAGE(BMS_MANAGER_RC_NULL_POINTER, code, "bms_manager_init should return BMS_MANAGER_RC_NULL_POINTER if the send_receive callback pointer is NULL");
}

void test_bms_manager_init_null_send(void) {
    struct BmsManagerHandler expected_handler = {
        .send = prv_bms_manager_send,
        .send_receive = send_receive,
        .ltc_handler = { .count = CELLBOARD_SEGMENT_LTC_COUNT }
    };

    for (size_t i = 0U; i < CELLBOARD_SEGMENT_LTC_COUNT; ++i) {
        expected_handler.requested_config[i].GPIO = 0b11111;
        expected_handler.requested_config[i].REFON = 1U;
    }

    enum BmsManagerReturnCode code = bms_manager_init(NULL, send_receive);
    TEST_ASSERT_EQUAL_MESSAGE(BMS_MANAGER_RC_OK, code, "bms_manager_init should return BMS_MANAGER_RC_OK if the handler is initialized correctly even if the send callback pointer is NULL");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(&expected_handler, &bms_handler, sizeof(expected_handler), "bms_manager_init should initialize the handler structure with the correct values even if the send callback pointer is NULL");
}

void test_bms_manager_write_configuration_correct_config(void) {
    send_fake.return_val = BMS_MANAGER_RC_OK;

    uint8_t expected_cmd[LTC6811_1_WRITE_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)] = { 0 };
    const size_t expected_byte_size = ltc6811_1_wrcfg_encode_broadcast(&bms_handler.ltc_handler, bms_handler.requested_config, expected_cmd);

    enum BmsManagerReturnCode code = bms_manager_write_configuration();
    TEST_ASSERT_EQUAL_MESSAGE(BMS_MANAGER_RC_OK, code, "bms_manager_write_configuration should return BMS_MANAGER_RC_OK or BMS_MANAGER_RC_BUSY if the configuration is sent correctly");
    TEST_ASSERT_EQUAL_MESSAGE(expected_byte_size, send_fake.arg1_val, "bms_manager_write_configuration should call the send callback with the correct byte size");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(expected_cmd, captured_data, expected_byte_size, "bms_manager_write_configuration should call the send callback with the correct command bytes");
}

void test_bms_manager_read_configuration(void) {

    send_receive_fake.return_val = BMS_MANAGER_RC_OK;

    uint8_t expected_cmd[LTC6811_1_READ_BUFFER_SIZE] = { 0 };
    const size_t expected_byte_size = ltc6811_1_rdcfg_encode_broadcast(&bms_handler.ltc_handler, expected_cmd);

    uint8_t payload[16] = {
        0x8D, 0x34, 0x12, 0xAB, 0x56, 0x78, 0x00, 0x00, 0x4B, 0x78, 0x56, 0xCD, 0x9A, 0xBC, 0x00, 0x00
    };

    append_pec(&payload[0]);
    append_pec(&payload[8]);

    memcpy(captured_out_data, payload, sizeof(captured_out_data));

    uint8_t expected_read[LTC6811_1_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)] = { 0 };

    memcpy(expected_read, payload, sizeof(payload));

    ltc6811_1_rdcfg_decode_broadcast(&bms_handler.ltc_handler, expected_read, bms_handler.actual_config);

    enum BmsManagerReturnCode code = bms_manager_read_configuration();

    TEST_ASSERT_EQUAL_MESSAGE(BMS_MANAGER_RC_OK, code, "bms_manager_read_configuration should return BMS_MANAGER_RC_OK or BMS_MANAGER_RC_BUSY if the configuration is read correctly");
    TEST_ASSERT_EQUAL_MESSAGE(expected_byte_size, send_receive_fake.arg2_val, "bms_manager_read_configuration should call the send_receive callback with the correct byte size");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(expected_cmd, captured_data, expected_byte_size, "bms_manager_read_configuration should call the send_receive callback with the correct command bytes");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(expected_read, captured_out_data, sizeof(expected_read), "bms_manager_read_configuration should call the send_receive callback with the correct output buffer");
}

enum BmsManagerReturnCode send_capture_data(uint8_t *const data, const size_t size) {
    if (size > sizeof(captured_data)) {
        return BMS_MANAGER_RC_ERROR;
    }
    memcpy(captured_data, data, size);
    return BMS_MANAGER_RC_OK;
}

enum BmsManagerReturnCode send_recieve_capture_data(uint8_t *data, uint8_t *out, const size_t size, const size_t out_size) {
    if (size > sizeof(captured_data) || out_size > sizeof(captured_data)) {
        return BMS_MANAGER_RC_ERROR;
    }
    memcpy(captured_data, data, size);
    memcpy(out, captured_out_data, out_size);
    return BMS_MANAGER_RC_OK;
}

void setUp() {
    identity_api_init(CELLBOARD_ID);
    RESET_FAKE(send);
    RESET_FAKE(send_receive);

    memset(captured_data, 0, sizeof(captured_data));
    memset(captured_out_data, 0, sizeof(captured_out_data));

    bms_manager_init(send, send_receive);

    send_fake.custom_fake = send_capture_data;
    send_receive_fake.custom_fake = send_recieve_capture_data;
}

void tearDown() {
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_bms_manager_init_correct_values);
    RUN_TEST(test_bms_manager_init_null_send_receive);
    RUN_TEST(test_bms_manager_init_null_send);
    RUN_TEST(test_bms_manager_write_configuration_correct_config);
    RUN_TEST(test_bms_manager_read_configuration);

    return UNITY_END();
}
