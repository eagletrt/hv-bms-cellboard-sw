/*!
 * \file test_can-comm.c
 * \date 2024-06-23
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Test functions for the can-comm module
 */

#include "unity.h"
#include "can-communication-api.h"
#include "identity-api.h"
#include "cellboard-def.h"
#include "fff.h"
DEFINE_FFF_GLOBALS;

#define CELLBOARD_ID CELLBOARD_ID_1

FAKE_VALUE_FUNC(enum CanCommunicationReturnCode, send, const struct CanCommunicationFrame *);
FAKE_VALUE_FUNC(enum CanCommunicationReturnCode, on_receive, struct CanCommunicationFrame *);

static struct CanCommunicationNetworkConfig configs[CAN_COMMUNICATION_NETWORK_COUNT];
static struct CanCommunicationFrame frame = { .id = 0x01U, .length = 0U };

// --- init ---

void test_can_comm_init_null_configs(void) {
    enum CanCommunicationReturnCode code = can_communication_api_init(NULL);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMMUNICATION_RC_NULL_POINTER, code, "can_communication_api_init should return CAN_COMMUNICATION_RC_NULL_POINTER when configs is NULL");
}

void test_can_comm_init_null_send(void) {
    configs[CAN_COMMUNICATION_NETWORK_BMS].send = NULL;

    enum CanCommunicationReturnCode code = can_communication_api_init(configs);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMMUNICATION_RC_NULL_POINTER, code, "can_communication_api_init should return CAN_COMMUNICATION_RC_NULL_POINTER when send callback is NULL");
}

void test_can_comm_init_ok(void) {
    enum CanCommunicationReturnCode code = can_communication_api_init(configs);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMMUNICATION_RC_OK, code, "can_communication_api_init should return CAN_COMMUNICATION_RC_OK with valid callbacks");
}

// --- rx add ---

void test_can_comm_rx_add_invalid_network(void) {
    enum CanCommunicationReturnCode code = can_communication_api_add_to_rx(CAN_COMMUNICATION_NETWORK_COUNT, &frame);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMMUNICATION_RC_INVALID_NETWORK, code, "can_communication_api_add_to_rx should return CAN_COMMUNICATION_RC_INVALID_NETWORK for an out-of-bounds network");
}

void test_can_comm_rx_add_null_data(void) {
    enum CanCommunicationReturnCode code = can_communication_api_add_to_rx(CAN_COMMUNICATION_NETWORK_BMS, NULL);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMMUNICATION_RC_NULL_POINTER, code, "can_communication_api_add_to_rx should return CAN_COMMUNICATION_RC_NULL_POINTER when frame is NULL");
}

void test_can_comm_rx_add_invalid_payload_size(void) {
    frame.length = CAN_COMMUNICATION_FRAME_DATA_SIZE + 1U;

    enum CanCommunicationReturnCode code = can_communication_api_add_to_rx(CAN_COMMUNICATION_NETWORK_BMS, &frame);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMMUNICATION_RC_INVALID_LENGTH, code, "can_communication_api_add_to_rx should return CAN_COMMUNICATION_RC_INVALID_LENGTH when length exceeds the maximum");
}

void test_can_comm_rx_add_ok(void) {
    on_receive_fake.return_val = CAN_COMMUNICATION_RC_OK;

    enum CanCommunicationReturnCode code = can_communication_api_add_to_rx(CAN_COMMUNICATION_NETWORK_BMS, &frame);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMMUNICATION_RC_OK, code, "can_communication_api_add_to_rx should return CAN_COMMUNICATION_RC_OK with valid parameters");

    can_communication_api_process_rx(CAN_COMMUNICATION_NETWORK_BMS);

    TEST_ASSERT_EQUAL_MESSAGE(1, on_receive_fake.call_count, "can_communication_api_add_to_rx should queue the frame so that it is dispatched exactly once");
}

// --- tx add ---

void test_can_comm_tx_add_invalid_network(void) {
    enum CanCommunicationReturnCode code = can_communication_api_add_to_tx(CAN_COMMUNICATION_NETWORK_COUNT, &frame);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMMUNICATION_RC_INVALID_NETWORK, code, "can_communication_api_add_to_tx should return CAN_COMMUNICATION_RC_INVALID_NETWORK for an out-of-bounds network");
}

void test_can_comm_tx_add_null_data(void) {
    enum CanCommunicationReturnCode code = can_communication_api_add_to_tx(CAN_COMMUNICATION_NETWORK_BMS, NULL);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMMUNICATION_RC_NULL_POINTER, code, "can_communication_api_add_to_tx should return CAN_COMMUNICATION_RC_NULL_POINTER when frame is NULL");
}

void test_can_comm_tx_add_invalid_payload_size(void) {
    frame.length = CAN_COMMUNICATION_FRAME_DATA_SIZE + 1U;

    enum CanCommunicationReturnCode code = can_communication_api_add_to_tx(CAN_COMMUNICATION_NETWORK_BMS, &frame);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMMUNICATION_RC_INVALID_LENGTH, code, "can_communication_api_add_to_tx should return CAN_COMMUNICATION_RC_INVALID_LENGTH when length exceeds the maximum");
}

void test_can_comm_tx_add_ok(void) {
    send_fake.return_val = CAN_COMMUNICATION_RC_OK;

    enum CanCommunicationReturnCode code = can_communication_api_add_to_tx(CAN_COMMUNICATION_NETWORK_BMS, &frame);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMMUNICATION_RC_OK, code, "can_communication_api_add_to_tx should return CAN_COMMUNICATION_RC_OK with valid parameters");

    can_communication_api_process_tx(CAN_COMMUNICATION_NETWORK_BMS);

    TEST_ASSERT_EQUAL_MESSAGE(1, send_fake.call_count, "can_communication_api_add_to_tx should queue the frame so that it is sent exactly once");
}

// --- process ---

void test_can_comm_process_tx_invalid_network(void) {
    enum CanCommunicationReturnCode code = can_communication_api_process_tx(CAN_COMMUNICATION_NETWORK_COUNT);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMMUNICATION_RC_INVALID_NETWORK, code, "can_communication_api_process_tx should return CAN_COMMUNICATION_RC_INVALID_NETWORK for an out-of-bounds network");
}

void test_can_comm_process_tx_transmission_error(void) {
    send_fake.return_val = CAN_COMMUNICATION_RC_TRANSMISSION_ERROR;
    can_communication_api_add_to_tx(CAN_COMMUNICATION_NETWORK_BMS, &frame);

    enum CanCommunicationReturnCode code = can_communication_api_process_tx(CAN_COMMUNICATION_NETWORK_BMS);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMMUNICATION_RC_TRANSMISSION_ERROR, code, "can_communication_api_process_tx should return CAN_COMMUNICATION_RC_TRANSMISSION_ERROR when the send callback fails");
}

void test_can_comm_process_rx_invalid_network(void) {
    enum CanCommunicationReturnCode code = can_communication_api_process_rx(CAN_COMMUNICATION_NETWORK_COUNT);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMMUNICATION_RC_INVALID_NETWORK, code, "can_communication_api_process_rx should return CAN_COMMUNICATION_RC_INVALID_NETWORK for an out-of-bounds network");
}

void test_can_comm_process_rx_handler_error(void) {
    on_receive_fake.return_val = CAN_COMMUNICATION_RC_RECEIVE_HANDLER_ERROR;
    can_communication_api_add_to_rx(CAN_COMMUNICATION_NETWORK_BMS, &frame);

    enum CanCommunicationReturnCode code = can_communication_api_process_rx(CAN_COMMUNICATION_NETWORK_BMS);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMMUNICATION_RC_RECEIVE_HANDLER_ERROR, code, "can_communication_api_process_rx should return CAN_COMMUNICATION_RC_RECEIVE_HANDLER_ERROR when the receive callback fails");
}

void setUp(void) {
    identity_api_init(CELLBOARD_ID);
    RESET_FAKE(send);
    RESET_FAKE(on_receive);
    configs[CAN_COMMUNICATION_NETWORK_BMS] = (struct CanCommunicationNetworkConfig){
        .send = send,
        .on_receive = on_receive,
    };
    frame = (struct CanCommunicationFrame){ .id = 0x01U, .length = 0U };
    can_communication_api_init(configs);
}

void tearDown(void) {
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_can_comm_init_null_configs);
    RUN_TEST(test_can_comm_init_null_send);
    RUN_TEST(test_can_comm_init_ok);

    RUN_TEST(test_can_comm_rx_add_invalid_network);
    RUN_TEST(test_can_comm_rx_add_null_data);
    RUN_TEST(test_can_comm_rx_add_invalid_payload_size);
    RUN_TEST(test_can_comm_rx_add_ok);

    RUN_TEST(test_can_comm_tx_add_invalid_network);
    RUN_TEST(test_can_comm_tx_add_null_data);
    RUN_TEST(test_can_comm_tx_add_invalid_payload_size);
    RUN_TEST(test_can_comm_tx_add_ok);

    RUN_TEST(test_can_comm_process_tx_invalid_network);
    RUN_TEST(test_can_comm_process_tx_transmission_error);
    RUN_TEST(test_can_comm_process_rx_invalid_network);
    RUN_TEST(test_can_comm_process_rx_handler_error);

    return UNITY_END();
}
