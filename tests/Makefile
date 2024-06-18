INCLUDES = 	-I ../Core/Inc/bms/ \
			-I ../Core/Inc/common/ \
			-I ../Core/Lib/can/lib/bms/ \
			-I ../Core/Lib/micro-libs/Unity/src/ \
			-I ../Core/Lib/micro-libs/CMock/src/ \
			-I ../Core/Lib/micro-libs/blinky/inc \
			-I mocks

BIN_DIR = bin

MOCKS =  $(BIN_DIR)/Mockidentity.o \
		 $(BIN_DIR)/Mockblinky.o

TESTS = test_led

test_all: $(TESTS)

$(BIN_DIR):
	mkdir $(BIN_DIR)

$(BIN_DIR)/unity.o:
	gcc -c ../Core/Lib/micro-libs/Unity/src/unity.c $(INCLUDES) -o $(BIN_DIR)/unity.o

$(BIN_DIR)/cmock.o:
	gcc -c ../Core/Lib/micro-libs/Cmock/src/cmock.c $(INCLUDES) -o $(BIN_DIR)/cmock.o

$(BIN_DIR)/%.o: mocks/%.c $(BIN_DIR) $(BIN_DIR)/unity.o $(BIN_DIR)/cmock.o 
	gcc -c $< -o $@ $(INCLUDES)

test_%: test_%.c $(MOCKS)
	gcc $< ../Core/Src/bms/$(patsubst test_%,%,$<) $(BIN_DIR)/*.o -o $@ $(INCLUDES)
	./$@

clean:
	rm -rf bin $(TESTS)

