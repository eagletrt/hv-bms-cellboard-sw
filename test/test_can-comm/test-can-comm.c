/**
 * @file test_can-comm.c
 * @date 2024-06-23
 * @author Riccardo Segala [riccardo.segala@icloud.com]
 *
 * @brief Test functions for the can-comm module
 */

#include "unity.h"
#include "can-comm.h"
#include "identity-api.h"
#include "cellboard-def.h"

#define CELLBOARD_ID CELLBOARD_ID_1

extern _CanCommHandler hcan_comm;

bool sended;
CanCommReturnCode can_comm_send(can_id_t id, CanFrameType frame_type, const uint8_t *data, size_t size) {
    sended = true;
    return CAN_COMM_OK;
}

void test_can_comm_init_null() {
    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_NULL_POINTER, can_comm_init(NULL), "Init should return CAN_COMM_NULL_POINTER when callback is NULL");
}

void test_can_comm_init_ok() {
    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_OK, can_comm_init(can_comm_send), "Init should return CAN_COMM_OK with valid callback");
}

void test_can_comm_enable_all() {
    can_comm_enable_all();
    TEST_ASSERT_EQUAL_MESSAGE(0x03, hcan_comm.enabled, "Enabled bitmask should be 0x03 (binary 11) after enabling all");
}

void test_can_comm_disable_all() {
    hcan_comm.enabled = 0x03;
    can_comm_disable_all();
    TEST_ASSERT_EQUAL_MESSAGE(0x00, hcan_comm.enabled, "Enabled bitmask should be 0x00 after disabling all");
}

void test_can_comm_is_enabled_all() {
    can_comm_enable_all();
    TEST_ASSERT_TRUE_MESSAGE(can_comm_is_enabled_all(),
                             "can_comm_is_enabled_all should return true when all bits are set");
}

void test_can_comm_enable() {
    can_comm_enable(0);
    can_comm_enable(1);
    TEST_ASSERT_EQUAL_MESSAGE(0x03, hcan_comm.enabled, "Enabled bitmask should be 0x03 after enabling indices 0 and 1 individually");
}

void test_can_comm_disable() {
    can_comm_enable_all();
    can_comm_disable(0);
    can_comm_disable(1);
    TEST_ASSERT_EQUAL_MESSAGE(0x00, hcan_comm.enabled, "Enabled bitmask should be 0x00 after disabling indices 0 and 1");
}

void test_can_comm_is_enabled_false() {
    TEST_ASSERT_FALSE_MESSAGE(can_comm_is_enabled(0),
                              "Should return false for index 0 when default initialized (disabled)");
}

void test_can_comm_is_enabled_true() {
    can_comm_enable(0);
    TEST_ASSERT_TRUE_MESSAGE(can_comm_is_enabled(0),
                             "Should return true for index 0 after calling enable(0)");
}

void test_can_comm_send_immediate_ok() {
    can_comm_enable_all();
    CanCommReturnCode ret = can_comm_send_immediate(0, CAN_FRAME_TYPE_DATA, (void *)0x01, 0);
    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_OK, ret, "Immediate send should return OK when enabled and parameters are valid");
}

void test_can_comm_send_immediate_sended() {
    can_comm_enable_all();
    can_comm_send_immediate(0, CAN_FRAME_TYPE_DATA, (void *)0x01, 0);
    TEST_ASSERT_TRUE_MESSAGE(sended,
                             "The 'sended' flag should be true, indicating the mock send function was called");
}

void test_can_comm_send_immediate_disabled() {
    CanCommReturnCode ret = can_comm_send_immediate(0, CAN_FRAME_TYPE_DATA, (void *)0x01, 0);
    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_DISABLED, ret, "Should return CAN_COMM_DISABLED when trying to send without enabling");
}

void test_can_comm_send_immediate_invalid_frame_type() {
    can_comm_enable_all();
    CanCommReturnCode ret = can_comm_send_immediate(0, CAN_FRAME_TYPE_COUNT + 1, (void *)0x01, 0);
    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_INVALID_FRAME_TYPE, ret, "Should return CAN_COMM_INVALID_FRAME_TYPE for out-of-bound frame type");
}

void test_can_comm_send_immediate_invalid_payload_size() {
    // can_comm_enable_all();
    // CanCommReturnCode ret = can_comm_send_immediate(0, CAN_FRAME_TYPE_DATA, (void *)0x01, CAN_COMM_MAX_PAYLOAD_BYTE_SIZE + 1);
    // TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_INVALID_PAYLOAD_SIZE, ret, "Should return CAN_COMM_INVALID_PAYLOAD_SIZE when size exceeds MAX");

    TEST_IGNORE_MESSAGE("Test for invalid payload size is currently disabled due to TODO in can_comm_send_immediate");
}

void test_can_comm_send_immediate_null() {
    can_comm_enable_all();
    CanCommReturnCode ret = can_comm_send_immediate(0, CAN_FRAME_TYPE_DATA, NULL, 0);
    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_NULL_POINTER, ret, "Should return CAN_COMM_NULL_POINTER when data pointer is NULL");
}

void test_can_comm_rx_add_disabled() {
    CanCommReturnCode ret = can_comm_rx_add(0, CAN_FRAME_TYPE_DATA, NULL, 0);
    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_DISABLED, ret, "RX add should fail with DISABLED when module is not enabled");
}

void test_can_comm_rx_add_invalid_index() {
    can_comm_enable_all();
    CanCommReturnCode ret = can_comm_rx_add(bms_MESSAGE_COUNT, CAN_FRAME_TYPE_DATA, NULL, 0);
    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_INVALID_INDEX, ret, "RX add should fail with INVALID_INDEX when index is out of bounds");
}

void test_can_comm_rx_add_null() {
    can_comm_enable_all();
    CanCommReturnCode ret = can_comm_rx_add(0, CAN_FRAME_TYPE_DATA, NULL, 0);
    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_NULL_POINTER, ret, "RX add should fail with NULL_POINTER when data is NULL");
}

void test_can_comm_rx_add_invalid_payload_size() {
    can_comm_enable_all();
    CanCommReturnCode ret = can_comm_rx_add(0, CAN_FRAME_TYPE_DATA, (void *)0x01, CAN_COMM_MAX_PAYLOAD_BYTE_SIZE + 1);
    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_INVALID_PAYLOAD_SIZE, ret, "RX add should fail with INVALID_PAYLOAD_SIZE when size is too large");
}

void test_can_comm_rx_add_invalid_frame() {
    can_comm_enable_all();
    CanCommReturnCode ret = can_comm_rx_add(0, CAN_FRAME_TYPE_COUNT + 1, (void *)0x01, 0);
    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_INVALID_FRAME_TYPE, ret, "RX add should fail with INVALID_FRAME_TYPE when type is unknown");
}

void test_can_comm_rx_add_ok() {
    can_comm_enable_all();
    CanCommReturnCode ret = can_comm_rx_add(0, CAN_FRAME_TYPE_DATA, (void *)0x01, 0);
    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_OK, ret, "RX add should return OK for valid parameters");
}

void test_can_comm_rx_add_added() {
    can_comm_enable_all();
    can_comm_rx_add(0, CAN_FRAME_TYPE_DATA, (void *)(0x01), 0);

    CanMessage rx_msg;
    TEST_ASSERT_EQUAL_MESSAGE(RING_BUFFER_OK, ring_buffer_pop_front(&hcan_comm.rx_buf, &rx_msg), "Should successfully pop the added message from the RX ring buffer");
}

void test_can_comm_rx_add_added_payload() {
    uint8_t data[] = { 0x01, 0x02, 0x03, 0x04 };

    can_comm_enable_all();
    can_comm_rx_add(0, CAN_FRAME_TYPE_DATA, data, 4);

    CanMessage rx_msg;
    ring_buffer_pop_front(&hcan_comm.rx_buf, &rx_msg);

    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(data, rx_msg.payload.rx, 4, "RX message payload content in buffer should match added data");
}

//TX

void test_can_comm_tx_add_disabled() {
    CanCommReturnCode ret = can_comm_tx_add(0, CAN_FRAME_TYPE_DATA, NULL, 0);
    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_DISABLED, ret, "TX add should fail with DISABLED when module is not enabled");
}

void test_can_comm_tx_add_invalid_index() {
    can_comm_enable_all();
    CanCommReturnCode ret = can_comm_tx_add(bms_MESSAGE_COUNT, CAN_FRAME_TYPE_DATA, NULL, 0);
    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_INVALID_INDEX, ret, "TX add should fail with INVALID_INDEX when index is out of bounds");
}

void test_can_comm_tx_add_null() {
    can_comm_enable_all();
    CanCommReturnCode ret = can_comm_tx_add(0, CAN_FRAME_TYPE_DATA, NULL, 0);
    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_NULL_POINTER, ret, "TX add should fail with NULL_POINTER when data is NULL");
}

void test_can_comm_tx_add_invalid_payload_size() {
    // can_comm_enable_all();
    // CanCommReturnCode ret = can_comm_tx_add(0, CAN_FRAME_TYPE_DATA, (void *)0x01, CAN_COMM_MAX_PAYLOAD_BYTE_SIZE + 1);
    // TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_INVALID_PAYLOAD_SIZE, ret, "TX add should fail with INVALID_PAYLOAD_SIZE when size is too large");
    TEST_IGNORE_MESSAGE("Test for invalid payload size is currently disabled due to TODO in can_comm_tx_add");
}

void test_can_comm_tx_add_invalid_frame() {
    can_comm_enable_all();
    CanCommReturnCode ret = can_comm_tx_add(0, CAN_FRAME_TYPE_COUNT + 1, (void *)0x01, 0);
    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_INVALID_FRAME_TYPE, ret, "TX add should fail with INVALID_FRAME_TYPE when type is unknown");
}

void test_can_comm_tx_add_ok() {
    can_comm_enable_all();
    CanCommReturnCode ret = can_comm_tx_add(0, CAN_FRAME_TYPE_DATA, (void *)0x01, 0);
    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_OK, ret, "TX add should return OK for valid parameters");
}

void test_can_comm_tx_add_added() {
    can_comm_enable_all();
    can_comm_tx_add(0, CAN_FRAME_TYPE_DATA, (void *)(0x01), 0);

    CanMessage tx_msg;
    TEST_ASSERT_EQUAL_MESSAGE(RING_BUFFER_OK, ring_buffer_pop_front(&hcan_comm.tx_buf, &tx_msg), "Should successfully pop the added message from the TX ring buffer");
}

void test_can_comm_tx_add_added_payload() {
    uint8_t data[] = { 0x01, 0x02, 0x03, 0x04 };

    can_comm_enable_all();
    can_comm_tx_add(0, CAN_FRAME_TYPE_DATA, data, 4);

    CanMessage tx_msg;
    ring_buffer_pop_front(&hcan_comm.tx_buf, &tx_msg);

    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(data, tx_msg.payload.tx, 4, "TX message payload content in buffer should match added data");
}

void setUp() {
    identity_api_init(CELLBOARD_ID);
    can_comm_init(can_comm_send);
    sended = false;
}

void tearDown() {
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_can_comm_init_null);
    RUN_TEST(test_can_comm_init_ok);
    RUN_TEST(test_can_comm_enable_all);
    RUN_TEST(test_can_comm_disable_all);
    RUN_TEST(test_can_comm_is_enabled_all);
    RUN_TEST(test_can_comm_enable);
    RUN_TEST(test_can_comm_disable);
    RUN_TEST(test_can_comm_is_enabled_false);
    RUN_TEST(test_can_comm_is_enabled_true);
    RUN_TEST(test_can_comm_send_immediate_ok);
    RUN_TEST(test_can_comm_send_immediate_sended);
    RUN_TEST(test_can_comm_send_immediate_disabled);
    RUN_TEST(test_can_comm_send_immediate_invalid_frame_type);
    RUN_TEST(test_can_comm_send_immediate_invalid_payload_size);
    RUN_TEST(test_can_comm_send_immediate_null);
    RUN_TEST(test_can_comm_rx_add_disabled);
    RUN_TEST(test_can_comm_rx_add_invalid_index);
    RUN_TEST(test_can_comm_rx_add_null);
    RUN_TEST(test_can_comm_rx_add_invalid_payload_size);
    RUN_TEST(test_can_comm_rx_add_invalid_frame);
    RUN_TEST(test_can_comm_rx_add_ok);
    RUN_TEST(test_can_comm_rx_add_added);
    RUN_TEST(test_can_comm_rx_add_added_payload);
    RUN_TEST(test_can_comm_tx_add_disabled);
    RUN_TEST(test_can_comm_tx_add_invalid_index);
    RUN_TEST(test_can_comm_tx_add_null);
    RUN_TEST(test_can_comm_tx_add_invalid_payload_size);
    RUN_TEST(test_can_comm_tx_add_invalid_frame);
    RUN_TEST(test_can_comm_tx_add_ok);
    RUN_TEST(test_can_comm_tx_add_added);
    RUN_TEST(test_can_comm_tx_add_added_payload);
    return UNITY_END();
}
