INCLUDES = 	-I ../Core/Inc/bms/ \
			-I ../Core/Inc/common/ \
			-I $(LIB_DIR)/can/lib/bms/ \
			-I $(LIB_DIR)/micro-libs/Unity/src/ \
			-I $(LIB_DIR)/micro-libs/CMock/src/ \
			-I $(LIB_DIR)/micro-libs/blinky/inc \
			-I mocks

LIB_DIR = ../Core/Lib
BMS_DIR = ../Core/src/bms
BIN_DIR = bin

FLAGS = -D TEST

MOCKS = $(BIN_DIR)/identity.o
OBJS = $(BIN_DIR)/blinky.o 

TESTS = test_led

test_all: $(TESTS)

$(BIN_DIR):
	mkdir $(BIN_DIR)

$(BIN_DIR)/unity.o: $(BIN_DIR)
	gcc -c ../Core/Lib/micro-libs/Unity/src/unity.c $(INCLUDES) -o $(BIN_DIR)/unity.o

$(BIN_DIR)/cmock.o: $(BIN_DIR)
	gcc -c ../Core/Lib/micro-libs/Cmock/src/cmock.c $(INCLUDES) -o $(BIN_DIR)/cmock.o

$(BIN_DIR)/%.o: mocks/Mock%.c $(BIN_DIR) $(BIN_DIR)/cmock.o
	gcc -c $< -o $@ $(INCLUDES)

$(BIN_DIR)/blinky.o: $(LIB_DIR)/micro-libs/blinky/src/blinky.c $(BIN_DIR)
	gcc -c $< $(INCLUDES) -o $@

test_%: test_%.c $(MOCKS) $(OBJS) $(BIN_DIR) $(BIN_DIR)/unity.o
	gcc $< ../Core/Src/bms/$(patsubst test_%,%,$<) -D TEST $(BIN_DIR)/*.o -o $@ $(INCLUDES)
	./$@

clean:
	rm -rf bin $(TESTS)

