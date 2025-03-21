UNAME := $(shell uname)

INCLUDES = 	-I $(INC_DIR)/bms/ \
			-I $(INC_DIR)/bms/timebase \
			-I $(INC_DIR)/bms/errors \
			-I $(INC_DIR)/common/ \
			-I $(INC_DIR) \
			-I $(MICRO_LIB_DIR)/bms-monitor/inc \
			-I $(LIB_DIR)/can/lib/bms/ \
			-I ../Drivers/STM32G4xx_HAL_Driver/Inc \
			-I ../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy \
			-I ../Drivers/CMSIS/Device/ST/STM32G4xx/Include \
			-I ../Drivers/CMSIS/Include \
			-I $(MICRO_LIB_DIR)/Unity/src/ \
			-I $(MICRO_LIB_DIR)/CMock/src/ \
			-I $(MICRO_LIB_DIR)/blinky/inc \
			-I $(MICRO_LIB_DIR)/ring-buffer/inc \
			-I $(MICRO_LIB_DIR)/min-heap/inc \
			-I $(MICRO_LIB_DIR)/bms-monitor/inc \
			-I $(LIB_DIR)/can/lib \
			-I $(LIB_DIR)/can/bms \
			-I mocks

ifeq ($(UNAME), Darwin)
	CC = aarch64-linux-musl-cc
	RUN = docker run -it --rm -v .:/home alpine /home/
else
	CC = gcc
	RUN = ./
endif

FLAGS = -D_STATIC="" -D_INLINE="" -DSTM32G4A1xx -fdata-sections -ffunction-sections -g -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast

LIB_DIR = ../Core/Lib
MICRO_LIB_DIR = $(LIB_DIR)/micro-libs

INC_DIR = ../Core/Inc
SRC_DIR = ../Core/Src
BMS_DIR = $(SRC_DIR)/bms

MOCKS_CONF = conf.yaml
MOCKS_DIR = mocks
MOCK_OBJS = $(BIN_DIR)/Mockadc.o \
			$(BIN_DIR)/Mockfdcan.o \
			$(BIN_DIR)/Mockgpio.o \
			$(BIN_DIR)/Mockspi.o \
			$(BIN_DIR)/Mockstm32g4xx_hal_conf.o \
			$(BIN_DIR)/Mockstm32g4xx_it.o \
			$(BIN_DIR)/Mocktim.o \
			$(BIN_DIR)/Mockusart.o

BMS_OBJS = $(BIN_DIR)/bal.o \
		   $(BIN_DIR)/bms-manager.o \
		   $(BIN_DIR)/can-comm.o \
		   $(BIN_DIR)/canlib-watchdog.o \
		   $(BIN_DIR)/fsm.o \
		   $(BIN_DIR)/identity.o \
		   $(BIN_DIR)/led.o \
		   $(BIN_DIR)/post.o \
		   $(BIN_DIR)/programmer.o \
		   $(BIN_DIR)/temp.o \
		   $(BIN_DIR)/volt.o \
		   $(BIN_DIR)/timebase/tasks.o \
		   $(BIN_DIR)/timebase/timebase.o \
		   $(BIN_DIR)/timebase/watchdog.o \
		   $(BIN_DIR)/errors/error.o

LIB_OBJS = $(BIN_DIR)/blinky.o \
		   $(BIN_DIR)/ring-buffer.o \
		   $(BIN_DIR)/min-heap.o \
		   $(BIN_DIR)/unity.o \
		   $(BIN_DIR)/cmock.o \
		   $(BIN_DIR)/bms_network.o \
		   $(BIN_DIR)/canlib_device.o \
		   $(BIN_DIR)/ltc6811.o

OBJS = $(BMS_OBJS) $(LIB_OBJS) $(MOCK_OBJS)

BIN_DIR = bin
BIN_TIMEBASE_DIR = $(BIN_DIR)/timebase
BIN_ERROR_DIR = $(BIN_DIR)/errors

TESTS = test_led \
		test_volt \
		test_bal \
		test_identity \
		test_bms-manager \
		test_can-comm \
		test_programmer


test_all: $(TESTS)

$(BIN_DIR):
	mkdir $(BIN_DIR)
	mkdir $(BIN_TIMEBASE_DIR)
	mkdir $(BIN_ERROR_DIR)

$(BIN_DIR)/unity.o: | $(BIN_DIR)
	$(CC) -c $(MICRO_LIB_DIR)/Unity/src/unity.c $(INCLUDES) -D UNITY_OUTPUT_COLOR -o $(BIN_DIR)/unity.o

$(BIN_DIR)/cmock.o: | $(BIN_DIR)
	$(CC) -c $(MICRO_LIB_DIR)/CMock/src/cmock.c $(INCLUDES) -o $(BIN_DIR)/cmock.o

$(BIN_DIR)/blinky.o: | $(BIN_DIR)
	$(CC) -c $(MICRO_LIB_DIR)/blinky/src/blinky.c $(INCLUDES) -o $(BIN_DIR)/blinky.o

$(BIN_DIR)/ring-buffer.o: | $(BIN_DIR)
	$(CC) -c $(MICRO_LIB_DIR)/ring-buffer/src/ring-buffer.c $(INCLUDES) -o $(BIN_DIR)/ring-buffer.o

$(BIN_DIR)/min-heap.o:| $(BIN_DIR)
	$(CC) -c $(MICRO_LIB_DIR)/min-heap/src/min-heap.c $(INCLUDES) -o $(BIN_DIR)/min-heap.o

$(BIN_DIR)/bms_network.o: | $(BIN_DIR)
	$(CC) -c $(LIB_DIR)/can/lib/bms/bms_network.c $(INCLUDES) -o $(BIN_DIR)/bms_network.o
	
$(BIN_DIR)/canlib_device.o: | $(BIN_DIR)
	$(CC) -c $(LIB_DIR)/can/lib/canlib_device.c $(INCLUDES) -o $(BIN_DIR)/canlib_device.o

$(BIN_DIR)/ltc6811.o: | $(BIN_DIR)
	$(CC) -c $(MICRO_LIB_DIR)/bms-monitor/src/ltc6811.c $(INCLUDES) -o $(BIN_DIR)/ltc6811.o

.PRECIOUS: $(MOCKS_DIR)/Mock%.c 
$(MOCKS_DIR)/Mock%.c: $(INC_DIR)/%.h $(MOCKS_CONF) 
	ruby $(MICRO_LIB_DIR)/CMock/lib/cmock.rb -o$(MOCKS_CONF) $< 

$(BIN_DIR)/Mock%.o: $(MOCKS_DIR)/Mock%.c $(BIN_DIR)/cmock.o | $(BIN_DIR)
	$(CC) -c $< -o $@ $(INCLUDES) $(FLAGS)

$(BIN_DIR)/%.o: $(BMS_DIR)/%.c | $(BIN_DIR) 
	$(CC) -c $< -o $@ $(INCLUDES) $(FLAGS)

.PRECIOUS: $(BIN_DIR)/%.o
.PRECIOUS: test_%
test_%: test_%.c $(OBJS) | $(BIN_DIR)
	$(CC) $< $(FLAGS) $(OBJS) -o $@ $(INCLUDES)

run_%: test_%
	$(RUN)$<

run_all: $(TESTS)
	$(foreach test, $(TESTS), $(RUN)$(test) || true;)

clean:
	rm -rf bin $(TESTS) $(MOCKS_DIR)

