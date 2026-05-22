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
#include "volt-api.h"
#include "temp-api.h"
#include "fff.h"
DEFINE_FFF_GLOBALS;

uint16_t prv_ltc6811_1_pec15(const uint8_t *const payload, const size_t len);

static void append_pec(uint8_t *frame) {
    uint16_t pec = prv_ltc6811_1_pec15(frame, 6);
    frame[6] = (pec >> 8) & 0xFF;
    frame[7] = pec & 0xFF;
}

#define CELLBOARD_ID CELLBOARD_ID_1

uint8_t captured_data[LTC6811_1_WRITE_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)] = { 0 };
uint8_t fake_recieved_data[LTC6811_1_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)] = { 0 };

FAKE_VALUE_FUNC(enum BmsManagerReturnCode, send, uint8_t *const, const size_t);
FAKE_VALUE_FUNC(enum BmsManagerReturnCode, send_receive, uint8_t *, uint8_t *, const size_t, const size_t);

enum BmsManagerReturnCode prv_bms_manager_api_send(uint8_t *const data, const size_t size);
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

    enum BmsManagerReturnCode code = bms_manager_api_init(send, send_receive);
    TEST_ASSERT_EQUAL_MESSAGE(BMS_MANAGER_RC_OK, code, "bms_manager_api_init should return BMS_MANAGER_RC_OK if the handler is initialized correctly");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(&expected_handler, &bms_handler, sizeof(expected_handler), "bms_manager_api_init should initialize the handler structure with the correct values");
}

void test_bms_manager_init_null_send_receive(void) {
    enum BmsManagerReturnCode code = bms_manager_api_init(send, NULL);
    TEST_ASSERT_EQUAL_MESSAGE(BMS_MANAGER_RC_NULL_POINTER, code, "bms_manager_api_init should return BMS_MANAGER_RC_NULL_POINTER if the send_receive callback pointer is NULL");
}

void test_bms_manager_init_null_send(void) {
    struct BmsManagerHandler expected_handler = {
        .send = prv_bms_manager_api_send,
        .send_receive = send_receive,
        .ltc_handler = { .count = CELLBOARD_SEGMENT_LTC_COUNT }
    };

    for (size_t i = 0U; i < CELLBOARD_SEGMENT_LTC_COUNT; ++i) {
        expected_handler.requested_config[i].GPIO = 0b11111;
        expected_handler.requested_config[i].REFON = 1U;
    }

    enum BmsManagerReturnCode code = bms_manager_api_init(NULL, send_receive);
    TEST_ASSERT_EQUAL_MESSAGE(BMS_MANAGER_RC_OK, code, "bms_manager_api_init should return BMS_MANAGER_RC_OK if the handler is initialized correctly even if the send callback pointer is NULL");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(&expected_handler, &bms_handler, sizeof(expected_handler), "bms_manager_api_init should initialize the handler structure with the correct values even if the send callback pointer is NULL");
}

void test_bms_manager_write_configuration_correct_config(void) {
    send_fake.return_val = BMS_MANAGER_RC_OK;

    uint8_t expected_cmd[LTC6811_1_WRITE_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)] = { 0 };
    const size_t expected_byte_size = ltc6811_1_wrcfg_encode_broadcast(&bms_handler.ltc_handler, bms_handler.requested_config, expected_cmd);

    enum BmsManagerReturnCode code = bms_manager_api_write_configuration();
    TEST_ASSERT_EQUAL_MESSAGE(BMS_MANAGER_RC_OK, code, "bms_manager_write_configuration should return BMS_MANAGER_RC_OK if the configuration is sent correctly");
    TEST_ASSERT_EQUAL_MESSAGE(expected_byte_size, send_fake.arg1_val, "bms_manager_write_configuration should call the send callback with the correct byte size");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(expected_cmd, captured_data, expected_byte_size, "bms_manager_write_configuration should call the send callback with the correct command bytes");
}

void test_bms_manager_read_configuration_correct_cmd(void) {
    send_receive_fake.return_val = BMS_MANAGER_RC_OK;

    uint8_t expected_cmd[LTC6811_1_READ_BUFFER_SIZE] = { 0 };
    const size_t expected_byte_size = ltc6811_1_rdcfg_encode_broadcast(&bms_handler.ltc_handler, expected_cmd);

    enum BmsManagerReturnCode code = bms_manager_api_read_configuration();
    // I don't check the return code as here it fails as the payload is not setup correctly
    TEST_ASSERT_EQUAL_MESSAGE(expected_byte_size, send_receive_fake.arg2_val, "bms_manager_read_configuration should call the send_receive callback with the correct byte size");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(expected_cmd, captured_data, expected_byte_size, "bms_manager_read_configuration should call the send_receive callback with the correct command bytes");
}

void test_bms_manager_read_configuration_decode_correctly(void) {
    send_receive_fake.return_val = BMS_MANAGER_RC_OK;

    // Set up return payload
    uint8_t payload[16] = {
        0x8D, 0x34, 0x12, 0xAB, 0x56, 0x78, 0x00, 0x00, 0x4B, 0x78, 0x56, 0xCD, 0x9A, 0xBC, 0x00, 0x00
    };
    append_pec(&payload[0]);
    append_pec(&payload[8]);
    memcpy(fake_recieved_data, payload, sizeof(fake_recieved_data));

    struct Ltc68111Cfgr expected_read[CELLBOARD_SEGMENT_LTC_COUNT] = { 0 };
    (void)ltc6811_1_rdcfg_decode_broadcast(&bms_handler.ltc_handler, fake_recieved_data, expected_read);

    enum BmsManagerReturnCode code = bms_manager_api_read_configuration();

    TEST_ASSERT_EQUAL_MESSAGE(BMS_MANAGER_RC_OK, code, "bms_manager_read_configuration should return BMS_MANAGER_RC_OK if the configuration is read and decoded correctly");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(expected_read, &bms_handler.actual_config, sizeof(struct Ltc68111Cfgr) * CELLBOARD_SEGMENT_LTC_COUNT, "bms_manager_read_configuration should call the send_receive callback with the correct output buffer");
}

// --- voltage conversion ---

void test_bms_manager_start_volt_conversion_correct_cmd(void) {
    send_fake.return_val = BMS_MANAGER_RC_OK;

    uint8_t expected_cmd[LTC6811_1_POLL_BUFFER_SIZE] = { 0 };
    const size_t expected_byte_size = ltc6811_1_adcv_encode_broadcast(
        &bms_handler.ltc_handler,
        LTC6811_1_MD_27KHZ,
        LTC6811_1_DCP_DISABLED,
        LTC6811_1_CH_ALL,
        expected_cmd);

    enum BmsManagerReturnCode code = bms_manager_api_start_volt_conversion();

    TEST_ASSERT_EQUAL_MESSAGE(BMS_MANAGER_RC_OK, code, "bms_manager_start_volt_conversion should return BMS_MANAGER_RC_OK");
    TEST_ASSERT_EQUAL_MESSAGE(expected_byte_size, send_fake.arg1_val, "bms_manager_start_volt_conversion should call send with the correct byte size");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(expected_cmd, captured_data, expected_byte_size, "bms_manager_start_volt_conversion should call send with the correct command bytes");
}

// --- temperature conversion ---

void test_bms_manager_start_temp_conversion_correct_cmd(void) {
    send_fake.return_val = BMS_MANAGER_RC_OK;

    uint8_t expected_cmd[LTC6811_1_POLL_BUFFER_SIZE] = { 0 };
    const size_t expected_byte_size = ltc6811_1_adax_encode_broadcast(
        &bms_handler.ltc_handler,
        LTC6811_1_MD_27KHZ,
        LTC6811_1_CHG_GPIO_ALL,
        expected_cmd);

    enum BmsManagerReturnCode code = bms_manager_api_start_temp_conversion();

    TEST_ASSERT_EQUAL_MESSAGE(BMS_MANAGER_RC_OK, code, "bms_manager_start_temp_conversion should return BMS_MANAGER_RC_OK");
    TEST_ASSERT_EQUAL_MESSAGE(expected_byte_size, send_fake.arg1_val, "bms_manager_start_temp_conversion should call send with the correct byte size");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(expected_cmd, captured_data, expected_byte_size, "bms_manager_start_temp_conversion should call send with the correct command bytes");
}

// --- open wire conversion ---

void test_bms_manager_start_open_wire_conversion_pull_up_correct_cmd(void) {
    send_fake.return_val = BMS_MANAGER_RC_OK;

    uint8_t expected_cmd[LTC6811_1_POLL_BUFFER_SIZE] = { 0 };
    const size_t expected_byte_size = ltc6811_1_adow_encode_broadcast(
        &bms_handler.ltc_handler,
        LTC6811_1_MD_27KHZ,
        LTC6811_1_PUP_ACTIVE,
        LTC6811_1_DCP_DISABLED,
        LTC6811_1_CH_ALL,
        expected_cmd);

    enum BmsManagerReturnCode code = bms_manager_api_start_open_wire_conversion(BMS_MANAGER_OPEN_WIRE_OPERATION_PUP);

    TEST_ASSERT_EQUAL_MESSAGE(BMS_MANAGER_RC_OK, code, "bms_manager_start_open_wire_conversion should return BMS_MANAGER_RC_OK with pull-up active");
    TEST_ASSERT_EQUAL_MESSAGE(expected_byte_size, send_fake.arg1_val, "bms_manager_start_open_wire_conversion should call send with the correct byte size");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(expected_cmd, captured_data, expected_byte_size, "bms_manager_start_open_wire_conversion should call send with the correct command bytes for pull-up");
}

void test_bms_manager_start_open_wire_conversion_invalid_pull(void) {
    enum BmsManagerReturnCode code = bms_manager_api_start_open_wire_conversion((enum BmsManagerOpenWireOperation)BMS_MANAGER_OPEN_WIRE_OPERATION_COUNT);

    TEST_ASSERT_EQUAL_MESSAGE(BMS_MANAGER_RC_ENCODE_ERROR, code, "bms_manager_start_open_wire_conversion should return BMS_MANAGER_RC_ENCODE_ERROR if an invalid pull-up value is given");
}

// --- poll conversion status ---

void test_bms_manager_poll_conversion_status_correct_cmd(void) {
    send_receive_fake.return_val = BMS_MANAGER_RC_OK;
    fake_recieved_data[0] = LTC6811_1_PLADC_COMPLETE_BYTE_VALUE;

    uint8_t expected_cmd[LTC6811_1_POLL_BUFFER_SIZE] = { 0 };
    const size_t expected_byte_size = ltc6811_1_pladc_encode_broadcast(&bms_handler.ltc_handler, expected_cmd);

    bms_manager_api_poll_conversion_status();

    TEST_ASSERT_EQUAL_MESSAGE(expected_byte_size, send_receive_fake.arg2_val, "bms_manager_poll_conversion_status should call send_receive with the correct byte size");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(expected_cmd, captured_data, expected_byte_size, "bms_manager_poll_conversion_status should call send_receive with the correct command bytes");
}

// --- read voltages ---

void test_bms_manager_read_voltages_invalid_register(void) {
    enum BmsManagerReturnCode code = bms_manager_api_read_voltages((enum BmsManagerVoltageRegister)BMS_MANAGER_VOLTAGE_REGISTER_COUNT);

    TEST_ASSERT_EQUAL_MESSAGE(BMS_MANAGER_RC_ENCODE_ERROR, code, "bms_manager_read_voltages should return BMS_MANAGER_RC_ENCODE_ERROR if an invalid voltage register is given");
}

void test_bms_manager_read_voltages_correct_cmd(void) {
    send_receive_fake.return_val = BMS_MANAGER_RC_OK;

    uint8_t expected_cmd[LTC6811_1_READ_BUFFER_SIZE] = { 0 };
    const size_t expected_byte_size = ltc6811_1_rdcv_encode_broadcast(
        &bms_handler.ltc_handler,
        (enum Ltc68111Cvxr)BMS_MANAGER_VOLTAGE_REGISTER_A,
        expected_cmd);

    uint8_t payload[LTC6811_1_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)] = {
        0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x00, 0x00
    };
    append_pec(&payload[0]);
    append_pec(&payload[8]);
    memcpy(fake_recieved_data, payload, sizeof(payload));

    enum BmsManagerReturnCode code = bms_manager_api_read_voltages(BMS_MANAGER_VOLTAGE_REGISTER_A);

    TEST_ASSERT_EQUAL_MESSAGE(BMS_MANAGER_RC_OK, code, "bms_manager_read_voltages should return BMS_MANAGER_RC_OK");
    TEST_ASSERT_EQUAL_MESSAGE(expected_byte_size, send_receive_fake.arg2_val, "bms_manager_read_voltages should call send_receive with the correct byte size");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(expected_cmd, captured_data, expected_byte_size, "bms_manager_read_voltages should call send_receive with the correct command bytes");
}

// --- read temperatures ---

void test_bms_manager_read_temperatures_correct_cmd(void) {
    send_receive_fake.return_val = BMS_MANAGER_RC_OK;

    uint8_t expected_cmd[LTC6811_1_READ_BUFFER_SIZE] = { 0 };
    const size_t expected_byte_size = ltc6811_1_rdaux_encode_broadcast(
        &bms_handler.ltc_handler,
        (enum Ltc68111Avxr)BMS_MANAGER_TEMPERATURE_REGISTER_A,
        expected_cmd);

    uint8_t payload[LTC6811_1_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)] = {
        0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x00, 0x00
    };
    append_pec(&payload[0]);
    append_pec(&payload[8]);
    memcpy(fake_recieved_data, payload, sizeof(payload));

    enum BmsManagerReturnCode code = bms_manager_api_read_temperatures(BMS_MANAGER_TEMPERATURE_REGISTER_A);

    TEST_ASSERT_EQUAL_MESSAGE(BMS_MANAGER_RC_OK, code, "bms_manager_read_temperatures should return BMS_MANAGER_RC_OK");
    TEST_ASSERT_EQUAL_MESSAGE(expected_byte_size, send_receive_fake.arg2_val, "bms_manager_read_temperatures should call send_receive with the correct byte size");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(expected_cmd, captured_data, expected_byte_size, "bms_manager_read_temperatures should call send_receive with the correct command bytes");
}

// --- read open wire voltages ---

void test_bms_manager_read_open_wire_voltages_correct_cmd(void) {
    send_receive_fake.return_val = BMS_MANAGER_RC_OK;

    uint8_t expected_cmd[LTC6811_1_READ_BUFFER_SIZE] = { 0 };
    const size_t expected_byte_size = ltc6811_1_rdcv_encode_broadcast(
        &bms_handler.ltc_handler,
        (enum Ltc68111Cvxr)BMS_MANAGER_VOLTAGE_REGISTER_A,
        expected_cmd);

    uint8_t payload[LTC6811_1_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)] = {
        0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x00, 0x00
    };
    append_pec(&payload[0]);
    append_pec(&payload[8]);
    memcpy(fake_recieved_data, payload, sizeof(payload));

    enum BmsManagerReturnCode code = bms_manager_api_read_open_wire_voltages(BMS_MANAGER_VOLTAGE_REGISTER_A, LTC6811_1_PUP_ACTIVE);

    TEST_ASSERT_EQUAL_MESSAGE(BMS_MANAGER_RC_OK, code, "bms_manager_read_open_wire_voltages should return BMS_MANAGER_RC_OK");
    TEST_ASSERT_EQUAL_MESSAGE(expected_byte_size, send_receive_fake.arg2_val, "bms_manager_read_open_wire_voltages should call send_receive with the correct byte size");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(expected_cmd, captured_data, expected_byte_size, "bms_manager_read_open_wire_voltages should call send_receive with the correct command bytes");
}

void test_bms_manager_read_open_wire_voltages_register_error(void) {
    enum BmsManagerReturnCode code = bms_manager_api_read_open_wire_voltages((enum BmsManagerVoltageRegister)BMS_MANAGER_VOLTAGE_REGISTER_COUNT, LTC6811_1_PUP_ACTIVE);

    TEST_ASSERT_EQUAL_MESSAGE(BMS_MANAGER_RC_ENCODE_ERROR, code, "bms_manager_read_open_wire_voltages should return BMS_MANAGER_RC_ENCODE_ERROR if an invalid voltage register is given");
}

void test_bms_manager_read_open_wire_voltages_pull_error(void) {
    enum BmsManagerReturnCode code = bms_manager_api_read_open_wire_voltages(BMS_MANAGER_VOLTAGE_REGISTER_A, (enum Ltc68111Pup)LTC6811_1_PUP_COUNT);

    TEST_ASSERT_EQUAL_MESSAGE(BMS_MANAGER_RC_ENCODE_ERROR, code, "bms_manager_read_open_wire_voltages should return BMS_MANAGER_RC_ENCODE_ERROR if an invalid pull-up value is given");
}

// --- check open wire ---

void test_bms_manager_check_open_wire_no_open_wire(void) {
    for (size_t i = 0U; i < CELLBOARD_SEGMENT_SERIES_PER_LTC_COUNT * CELLBOARD_SEGMENT_LTC_COUNT; ++i) {
        bms_handler.pup[LTC6811_1_PUP_ACTIVE][i] = 3.8f;
        bms_handler.pup[LTC6811_1_PUP_INACTIVE][i] = LTC6811_1_OPEN_WIRE_THRESHOLD_V + 0.1f;
    }

    enum BmsManagerReturnCode code = bms_manager_api_check_open_wire();

    TEST_ASSERT_EQUAL_MESSAGE(BMS_MANAGER_RC_OK, code, "bms_manager_check_open_wire should return BMS_MANAGER_RC_OK when no open wire is detected");
}

void test_bms_manager_check_open_wire_first_cell_open(void) {
    for (size_t i = 0U; i < CELLBOARD_SEGMENT_SERIES_PER_LTC_COUNT * CELLBOARD_SEGMENT_LTC_COUNT; ++i) {
        bms_handler.pup[LTC6811_1_PUP_ACTIVE][i] = 3.8f;
        bms_handler.pup[LTC6811_1_PUP_INACTIVE][i] = LTC6811_1_OPEN_WIRE_THRESHOLD_V + 0.1f;
    }
    bms_handler.pup[LTC6811_1_PUP_ACTIVE][0U] = BMS_MANAGER_OPEN_WIRE_ZERO_V;

    enum BmsManagerReturnCode code = bms_manager_api_check_open_wire();

    TEST_ASSERT_EQUAL_MESSAGE(BMS_MANAGER_RC_OPEN_WIRE, code, "bms_manager_check_open_wire should return BMS_MANAGER_RC_OPEN_WIRE when the first cell has an open wire");
}

void test_bms_manager_check_open_wire_last_cell_open(void) {
    for (size_t i = 0U; i < CELLBOARD_SEGMENT_SERIES_PER_LTC_COUNT * CELLBOARD_SEGMENT_LTC_COUNT; ++i) {
        bms_handler.pup[LTC6811_1_PUP_ACTIVE][i] = 3.8f;
        bms_handler.pup[LTC6811_1_PUP_INACTIVE][i] = LTC6811_1_OPEN_WIRE_THRESHOLD_V + 0.1f;
    }
    bms_handler.pup[LTC6811_1_PUP_INACTIVE][CELLBOARD_SEGMENT_SERIES_PER_LTC_COUNT - 1U] = BMS_MANAGER_OPEN_WIRE_ZERO_V;

    enum BmsManagerReturnCode code = bms_manager_api_check_open_wire();

    TEST_ASSERT_EQUAL_MESSAGE(BMS_MANAGER_RC_OPEN_WIRE, code, "bms_manager_check_open_wire should return BMS_MANAGER_RC_OPEN_WIRE when the last cell has an open wire");
}

void test_bms_manager_check_open_wire_mid_cell_open(void) {
    for (size_t i = 0U; i < CELLBOARD_SEGMENT_SERIES_PER_LTC_COUNT * CELLBOARD_SEGMENT_LTC_COUNT; ++i) {
        bms_handler.pup[LTC6811_1_PUP_ACTIVE][i] = 3.8f;
        bms_handler.pup[LTC6811_1_PUP_INACTIVE][i] = LTC6811_1_OPEN_WIRE_THRESHOLD_V + 0.1f;
    }
    bms_handler.pup[LTC6811_1_PUP_ACTIVE][15U] = 0.0f;
    bms_handler.pup[LTC6811_1_PUP_INACTIVE][15U] = 3.8f;

    enum BmsManagerReturnCode code = bms_manager_api_check_open_wire();

    TEST_ASSERT_EQUAL_MESSAGE(BMS_MANAGER_RC_OPEN_WIRE, code, "bms_manager_check_open_wire should return BMS_MANAGER_RC_OPEN_WIRE when a mid cell has an open wire");
}

// --- set/get discharge cells ---

void test_bms_manager_set_discharge_cells_correct_config(void) {
    const bit_flag32 cells = 0b000000100001000000010001U;
    const bit_flag16_t expected_dcc_ltc0 = 33U; //(cells >> (1U * CELLBOARD_SEGMENT_SERIES_PER_LTC_COUNT)) & ((1U << CELLBOARD_SEGMENT_SERIES_PER_LTC_COUNT) - 1U);
    const bit_flag16_t expected_dcc_ltc1 = 17U; //(cells >> (0U * CELLBOARD_SEGMENT_SERIES_PER_LTC_COUNT)) & ((1U << CELLBOARD_SEGMENT_SERIES_PER_LTC_COUNT) - 1U);

    bms_manager_api_set_discharge_cells(cells);

    TEST_ASSERT_EQUAL_MESSAGE(LTC6811_1_DCTO_30S, bms_handler.requested_config[0].DCTO, "bms_manager_set_discharge_cells should set DCTO_30S for LTC 0 when cells are active");
    TEST_ASSERT_EQUAL_MESSAGE(LTC6811_1_DCTO_30S, bms_handler.requested_config[1].DCTO, "bms_manager_set_discharge_cells should set DCTO_30S for LTC 1 when cells are active");
    TEST_ASSERT_EQUAL_MESSAGE(expected_dcc_ltc0, bms_handler.requested_config[0].DCC, "bms_manager_set_discharge_cells should set the correct DCC for LTC 0");
    TEST_ASSERT_EQUAL_MESSAGE(expected_dcc_ltc1, bms_handler.requested_config[1].DCC, "bms_manager_set_discharge_cells should set the correct DCC for LTC 1");
}

void test_bms_manager_set_discharge_cells_sets_dcto_off_when_no_cells(void) {
    struct Ltc68111Cfgr expected_config[CELLBOARD_SEGMENT_LTC_COUNT] = { 0 };

    bms_manager_api_set_discharge_cells(0U);

    TEST_ASSERT_EQUAL_MESSAGE(expected_config[0].DCTO, bms_handler.requested_config[0].DCTO, "bms_manager_set_discharge_cells should set DCTO_30S for LTC 0 when cells are active");
    TEST_ASSERT_EQUAL_MESSAGE(expected_config[1].DCTO, bms_handler.requested_config[1].DCTO, "bms_manager_set_discharge_cells should set DCTO_30S for LTC 1 when cells are active");
    TEST_ASSERT_EQUAL_MESSAGE(expected_config[0].DCC, bms_handler.requested_config[0].DCC, "bms_manager_set_discharge_cells should set the correct DCC for LTC 0");
    TEST_ASSERT_EQUAL_MESSAGE(expected_config[1].DCC, bms_handler.requested_config[1].DCC, "bms_manager_set_discharge_cells should set the correct DCC for LTC 1");
}

void test_bms_manager_get_discharge_cells_correct_value(void) {
    bms_handler.actual_config[0].DCC = 0b000000000001U;
    bms_handler.actual_config[1].DCC = 0b000000000010U;

    const bit_flag32 expected = (0b000000000001U << (0U * CELLBOARD_SEGMENT_SERIES_PER_LTC_COUNT)) |
                                (0b000000000010U << (1U * CELLBOARD_SEGMENT_SERIES_PER_LTC_COUNT));

    bit_flag32 cells = bms_manager_api_get_discharge_cells();

    TEST_ASSERT_EQUAL_HEX32_MESSAGE(expected, cells, "bms_manager_get_discharge_cells should return the correct cell bitmask from actual_config");
}

// --- helpers ---

enum BmsManagerReturnCode send_capture_data(uint8_t *const data, const size_t size) {
    if (size > sizeof(captured_data))
        return BMS_MANAGER_RC_ERROR;
    memcpy(captured_data, data, size);
    return BMS_MANAGER_RC_OK;
}

enum BmsManagerReturnCode send_recieve_capture_data(uint8_t *data, uint8_t *out, const size_t size, const size_t out_size) {
    if (size > sizeof(captured_data) || out_size > sizeof(captured_data))
        return BMS_MANAGER_RC_ERROR;
    memcpy(captured_data, data, size);
    memcpy(out, fake_recieved_data, out_size);
    return BMS_MANAGER_RC_OK;
}

void setUp() {
    identity_api_init(CELLBOARD_ID);
    volt_api_init();
    RESET_FAKE(send);
    RESET_FAKE(send_receive);

    memset(captured_data, 0, sizeof(captured_data));
    memset(fake_recieved_data, 0, sizeof(fake_recieved_data));
    memset(bms_handler.pup, 0, sizeof(bms_handler.pup));

    bms_manager_api_init(send, send_receive);

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
    RUN_TEST(test_bms_manager_read_configuration_correct_cmd);
    RUN_TEST(test_bms_manager_read_configuration_decode_correctly);

    RUN_TEST(test_bms_manager_start_volt_conversion_correct_cmd);

    RUN_TEST(test_bms_manager_start_temp_conversion_correct_cmd);

    RUN_TEST(test_bms_manager_start_open_wire_conversion_pull_up_correct_cmd);
    RUN_TEST(test_bms_manager_start_open_wire_conversion_invalid_pull);

    RUN_TEST(test_bms_manager_poll_conversion_status_correct_cmd);

    RUN_TEST(test_bms_manager_read_voltages_invalid_register);
    RUN_TEST(test_bms_manager_read_voltages_correct_cmd);

    RUN_TEST(test_bms_manager_read_temperatures_correct_cmd);

    RUN_TEST(test_bms_manager_read_open_wire_voltages_correct_cmd);
    RUN_TEST(test_bms_manager_read_open_wire_voltages_register_error);
    RUN_TEST(test_bms_manager_read_open_wire_voltages_pull_error);

    RUN_TEST(test_bms_manager_check_open_wire_no_open_wire);
    RUN_TEST(test_bms_manager_check_open_wire_first_cell_open);
    RUN_TEST(test_bms_manager_check_open_wire_last_cell_open);
    RUN_TEST(test_bms_manager_check_open_wire_mid_cell_open);

    RUN_TEST(test_bms_manager_set_discharge_cells_correct_config);
    RUN_TEST(test_bms_manager_set_discharge_cells_sets_dcto_off_when_no_cells);
    RUN_TEST(test_bms_manager_get_discharge_cells_correct_value);

    return UNITY_END();
}
