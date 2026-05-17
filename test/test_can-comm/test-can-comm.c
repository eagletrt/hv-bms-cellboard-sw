/*!
 * \file test_can-comm.c
 * \date 2024-06-23
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Test functions for the can-comm module
 */

#include "unity.h"
#include "can-comm-api.h"
#include "identity-api.h"
#include "cellboard-def.h"
#include "fff.h"
DEFINE_FFF_GLOBALS;

#define CELLBOARD_ID CELLBOARD_ID_1

FAKE_VALUE_FUNC(enum CanCommReturnCode, send, can_id_t, CanFrameType, const uint8_t *, size_t);

extern struct CanCommHandler can_comm_handler;

// --- init ---

void test_can_comm_init_null_send(void) {
    enum CanCommReturnCode code = can_comm_init(NULL);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_RC_NULL_POINTER, code, "can_comm_init should return CAN_COMM_RC_NULL_POINTER when send callback is NULL");
}

void test_can_comm_init_ok(void) {
    enum CanCommReturnCode code = can_comm_init(send);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_RC_OK, code, "can_comm_init should return CAN_COMM_RC_OK with a valid send callback");
    TEST_ASSERT_EQUAL_MESSAGE(0x00, can_comm_handler.enabled, "can_comm_init should leave the handler in the disabled state");
}

// --- enable/disable all ---

void test_can_comm_enable_all(void) {
    can_comm_enable_all();

    TEST_ASSERT_EQUAL_MESSAGE(0x03, can_comm_handler.enabled, "can_comm_enable_all should set the enabled bitmask to 0x03");
}

void test_can_comm_disable_all(void) {
    can_comm_enable_all();
    can_comm_disable_all();

    TEST_ASSERT_EQUAL_MESSAGE(0x00, can_comm_handler.enabled, "can_comm_disable_all should clear the enabled bitmask to 0x00");
}

void test_can_comm_is_enabled_all_true(void) {
    can_comm_enable_all();

    TEST_ASSERT_TRUE_MESSAGE(can_comm_is_enabled_all(), "can_comm_is_enabled_all should return true when all bits are set");
}

void test_can_comm_is_enabled_all_false(void) {
    can_comm_disable_all();

    TEST_ASSERT_FALSE_MESSAGE(can_comm_is_enabled_all(), "can_comm_is_enabled_all should return false when not all bits are set");
}

// --- enable/disable single bit ---

void test_can_comm_enable_sets_bits(void) {
    can_comm_enable(0);
    can_comm_enable(1);

    TEST_ASSERT_EQUAL_MESSAGE(0x03, can_comm_handler.enabled, "can_comm_enable should set each bit individually");
}

void test_can_comm_disable_clears_bits(void) {
    can_comm_enable_all();
    can_comm_disable(0);
    can_comm_disable(1);

    TEST_ASSERT_EQUAL_MESSAGE(0x00, can_comm_handler.enabled, "can_comm_disable should clear each bit individually");
}

void test_can_comm_is_enabled_false(void) {
    TEST_ASSERT_FALSE_MESSAGE(can_comm_is_enabled(0), "can_comm_is_enabled should return false when the bit is not set");
}

void test_can_comm_is_enabled_true(void) {
    can_comm_enable(0);

    TEST_ASSERT_TRUE_MESSAGE(can_comm_is_enabled(0), "can_comm_is_enabled should return true after enabling that bit");
}

// --- send immediate ---

void test_can_comm_send_immediate_disabled(void) {
    can_comm_disable_all();

    enum CanCommReturnCode code = can_comm_send_immediate(0, CAN_FRAME_TYPE_DATA, (void *)0x01, 0);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_RC_DISABLED, code, "can_comm_send_immediate should return CAN_COMM_RC_DISABLED when TX is not enabled");
}

void test_can_comm_send_immediate_invalid_index(void) {
    can_comm_enable_all();

    enum CanCommReturnCode code = can_comm_send_immediate(bms_MESSAGE_COUNT, CAN_FRAME_TYPE_DATA, (void *)0x01, 0);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_RC_INVALID_INDEX, code, "can_comm_send_immediate should return CAN_COMM_RC_INVALID_INDEX for an out-of-bounds index");
}

void test_can_comm_send_immediate_invalid_frame_type(void) {
    can_comm_enable_all();

    enum CanCommReturnCode code = can_comm_send_immediate(0, CAN_FRAME_TYPE_COUNT + 1, (void *)0x01, 0);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_RC_INVALID_FRAME_TYPE, code, "can_comm_send_immediate should return CAN_COMM_RC_INVALID_FRAME_TYPE for an invalid frame type");
}

void test_can_comm_send_immediate_null_data(void) {
    can_comm_enable_all();

    enum CanCommReturnCode code = can_comm_send_immediate(0, CAN_FRAME_TYPE_DATA, NULL, 0);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_RC_NULL_POINTER, code, "can_comm_send_immediate should return CAN_COMM_RC_NULL_POINTER when data is NULL");
}

void test_can_comm_send_immediate_ok(void) {
    send_fake.return_val = CAN_COMM_RC_OK;
    can_comm_enable_all();

    enum CanCommReturnCode code = can_comm_send_immediate(0, CAN_FRAME_TYPE_DATA, (void *)0x01, 0);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_RC_OK, code, "can_comm_send_immediate should return CAN_COMM_RC_OK with valid parameters");
}

void test_can_comm_send_immediate_calls_send(void) {
    send_fake.return_val = CAN_COMM_RC_OK;
    can_comm_enable_all();

    can_comm_send_immediate(0, CAN_FRAME_TYPE_DATA, (void *)0x01, 0);

    TEST_ASSERT_EQUAL_MESSAGE(1, send_fake.call_count, "can_comm_send_immediate should invoke the send callback exactly once");
}

void test_can_comm_send_immediate_invalid_payload_size(void) {
    TEST_IGNORE_MESSAGE("Test for invalid payload size is currently disabled due to TODO in can_comm_send_immediate");
}

// --- rx add ---

void test_can_comm_rx_add_disabled(void) {
    can_comm_disable_all();

    enum CanCommReturnCode code = can_comm_rx_add(0, CAN_FRAME_TYPE_DATA, (void *)0x01, 0);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_RC_DISABLED, code, "can_comm_rx_add should return CAN_COMM_RC_DISABLED when RX is not enabled");
}

void test_can_comm_rx_add_invalid_index(void) {
    can_comm_enable_all();

    enum CanCommReturnCode code = can_comm_rx_add(bms_MESSAGE_COUNT, CAN_FRAME_TYPE_DATA, (void *)0x01, 0);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_RC_INVALID_INDEX, code, "can_comm_rx_add should return CAN_COMM_RC_INVALID_INDEX for an out-of-bounds index");
}

void test_can_comm_rx_add_null_data(void) {
    can_comm_enable_all();

    enum CanCommReturnCode code = can_comm_rx_add(0, CAN_FRAME_TYPE_DATA, NULL, 0);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_RC_NULL_POINTER, code, "can_comm_rx_add should return CAN_COMM_RC_NULL_POINTER when data is NULL");
}

void test_can_comm_rx_add_invalid_payload_size(void) {
    can_comm_enable_all();

    enum CanCommReturnCode code = can_comm_rx_add(0, CAN_FRAME_TYPE_DATA, (void *)0x01, CAN_COMM_MAX_PAYLOAD_BYTE_SIZE + 1);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_RC_INVALID_PAYLOAD_SIZE, code, "can_comm_rx_add should return CAN_COMM_RC_INVALID_PAYLOAD_SIZE when size exceeds the maximum");
}

void test_can_comm_rx_add_invalid_frame_type(void) {
    can_comm_enable_all();

    enum CanCommReturnCode code = can_comm_rx_add(0, CAN_FRAME_TYPE_COUNT + 1, (void *)0x01, 0);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_RC_INVALID_FRAME_TYPE, code, "can_comm_rx_add should return CAN_COMM_RC_INVALID_FRAME_TYPE for an invalid frame type");
}

void test_can_comm_rx_add_ok(void) {
    can_comm_enable_all();

    enum CanCommReturnCode code = can_comm_rx_add(0, CAN_FRAME_TYPE_DATA, (void *)0x01, 0);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_RC_OK, code, "can_comm_rx_add should return CAN_COMM_RC_OK with valid parameters");
}

void test_can_comm_rx_add_message_in_buffer(void) {
    can_comm_enable_all();
    can_comm_rx_add(0, CAN_FRAME_TYPE_DATA, (void *)0x01, 0);

    struct CanMessage rx_msg;
    enum RingBufferReturnCode rc = ring_buffer_api_pop_front(&can_comm_handler.rx_buf, &rx_msg);

    TEST_ASSERT_EQUAL_MESSAGE(RING_BUFFER_RC_OK, rc, "can_comm_rx_add should push the message into the RX ring buffer");
}

void test_can_comm_rx_add_payload_stored_correctly(void) {
    uint8_t data[] = { 0x01, 0x02, 0x03, 0x04 };
    can_comm_enable_all();
    can_comm_rx_add(0, CAN_FRAME_TYPE_DATA, data, sizeof(data));

    struct CanMessage rx_msg;
    ring_buffer_api_pop_front(&can_comm_handler.rx_buf, &rx_msg);

    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(data, rx_msg.payload.rx, sizeof(data), "can_comm_rx_add should store the payload verbatim in the RX ring buffer");
}

void test_can_comm_rx_add_sets_busy_flag(void) {
    can_comm_enable_all();
    can_comm_rx_add(0, CAN_FRAME_TYPE_DATA, (void *)0x01, 0);

    TEST_ASSERT_TRUE_MESSAGE(can_comm_handler.rx_busy[0], "can_comm_rx_add should set the rx_busy flag for the given index");
}

// --- tx add ---

void test_can_comm_tx_add_disabled(void) {
    can_comm_disable_all();

    enum CanCommReturnCode code = can_comm_tx_add(0, CAN_FRAME_TYPE_DATA, (void *)0x01, 0);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_RC_DISABLED, code, "can_comm_tx_add should return CAN_COMM_RC_DISABLED when TX is not enabled");
}

void test_can_comm_tx_add_invalid_index(void) {
    can_comm_enable_all();

    enum CanCommReturnCode code = can_comm_tx_add(bms_MESSAGE_COUNT, CAN_FRAME_TYPE_DATA, (void *)0x01, 0);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_RC_INVALID_INDEX, code, "can_comm_tx_add should return CAN_COMM_RC_INVALID_INDEX for an out-of-bounds index");
}

void test_can_comm_tx_add_null_data(void) {
    can_comm_enable_all();

    enum CanCommReturnCode code = can_comm_tx_add(0, CAN_FRAME_TYPE_DATA, NULL, 0);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_RC_NULL_POINTER, code, "can_comm_tx_add should return CAN_COMM_RC_NULL_POINTER when data is NULL");
}

void test_can_comm_tx_add_invalid_frame_type(void) {
    can_comm_enable_all();

    enum CanCommReturnCode code = can_comm_tx_add(0, CAN_FRAME_TYPE_COUNT + 1, (void *)0x01, 0);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_RC_INVALID_FRAME_TYPE, code, "can_comm_tx_add should return CAN_COMM_RC_INVALID_FRAME_TYPE for an invalid frame type");
}

void test_can_comm_tx_add_ok(void) {
    can_comm_enable_all();

    enum CanCommReturnCode code = can_comm_tx_add(0, CAN_FRAME_TYPE_DATA, (void *)0x01, 0);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMM_RC_OK, code, "can_comm_tx_add should return CAN_COMM_RC_OK with valid parameters");
}

void test_can_comm_tx_add_message_in_buffer(void) {
    can_comm_enable_all();
    can_comm_tx_add(0, CAN_FRAME_TYPE_DATA, (void *)0x01, 0);

    struct CanMessage tx_msg;
    enum RingBufferReturnCode rc = ring_buffer_api_pop_front(&can_comm_handler.tx_buf, &tx_msg);

    TEST_ASSERT_EQUAL_MESSAGE(RING_BUFFER_RC_OK, rc, "can_comm_tx_add should push the message into the TX ring buffer");
}

void test_can_comm_tx_add_payload_stored_correctly(void) {
    uint8_t data[] = { 0x01, 0x02, 0x03, 0x04 };
    can_comm_enable_all();
    can_comm_tx_add(0, CAN_FRAME_TYPE_DATA, data, sizeof(data));

    struct CanMessage tx_msg;
    ring_buffer_api_pop_front(&can_comm_handler.tx_buf, &tx_msg);

    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(data, tx_msg.payload.tx, sizeof(data), "can_comm_tx_add should store the payload verbatim in the TX ring buffer");
}

void test_can_comm_tx_add_sets_busy_flag(void) {
    can_comm_enable_all();
    can_comm_tx_add(0, CAN_FRAME_TYPE_DATA, (void *)0x01, 0);

    TEST_ASSERT_TRUE_MESSAGE(can_comm_handler.tx_busy[0], "can_comm_tx_add should set the tx_busy flag for the given index");
}

void test_can_comm_tx_add_invalid_payload_size(void) {
    TEST_IGNORE_MESSAGE("Test for invalid payload size is currently disabled due to TODO in can_comm_tx_add");
}

void setUp(void) {
    identity_api_init(CELLBOARD_ID);
    RESET_FAKE(send);
    can_comm_init(send);
}

void tearDown(void) {
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_can_comm_init_null_send);
    RUN_TEST(test_can_comm_init_ok);
    RUN_TEST(test_can_comm_init_disabled);

    RUN_TEST(test_can_comm_enable_all);
    RUN_TEST(test_can_comm_disable_all);
    RUN_TEST(test_can_comm_is_enabled_all_true);
    RUN_TEST(test_can_comm_is_enabled_all_false);

    RUN_TEST(test_can_comm_enable_sets_bits);
    RUN_TEST(test_can_comm_disable_clears_bits);
    RUN_TEST(test_can_comm_is_enabled_false);
    RUN_TEST(test_can_comm_is_enabled_true);

    RUN_TEST(test_can_comm_send_immediate_disabled);
    RUN_TEST(test_can_comm_send_immediate_invalid_index);
    RUN_TEST(test_can_comm_send_immediate_invalid_frame_type);
    RUN_TEST(test_can_comm_send_immediate_null_data);
    RUN_TEST(test_can_comm_send_immediate_ok);
    RUN_TEST(test_can_comm_send_immediate_calls_send);
    RUN_TEST(test_can_comm_send_immediate_invalid_payload_size);

    RUN_TEST(test_can_comm_rx_add_disabled);
    RUN_TEST(test_can_comm_rx_add_invalid_index);
    RUN_TEST(test_can_comm_rx_add_null_data);
    RUN_TEST(test_can_comm_rx_add_invalid_payload_size);
    RUN_TEST(test_can_comm_rx_add_invalid_frame_type);
    RUN_TEST(test_can_comm_rx_add_ok);
    RUN_TEST(test_can_comm_rx_add_message_in_buffer);
    RUN_TEST(test_can_comm_rx_add_payload_stored_correctly);
    RUN_TEST(test_can_comm_rx_add_sets_busy_flag);

    RUN_TEST(test_can_comm_tx_add_disabled);
    RUN_TEST(test_can_comm_tx_add_invalid_index);
    RUN_TEST(test_can_comm_tx_add_null_data);
    RUN_TEST(test_can_comm_tx_add_invalid_frame_type);
    RUN_TEST(test_can_comm_tx_add_ok);
    RUN_TEST(test_can_comm_tx_add_message_in_buffer);
    RUN_TEST(test_can_comm_tx_add_payload_stored_correctly);
    RUN_TEST(test_can_comm_tx_add_sets_busy_flag);
    RUN_TEST(test_can_comm_tx_add_invalid_payload_size);

    return UNITY_END();
}