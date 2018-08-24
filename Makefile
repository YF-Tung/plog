BIN := plog
CC := g++
CFLAGS := --std=c++11 -Ofast

SRC_DIR := src
BIN_DIR := build
TEST_DIR := test
OBJ_DIR := obj
GTEST_DIR := googletest/googletest

# Binaries
plog:    plog.o
	@mkdir -p ${BIN_DIR}
	${CC} ${CFLAGS} ${OBJ_DIR}/$^ -o ${BIN_DIR}/$@

# Objs
plog.o: ${SRC_DIR}/plog.cpp
	@mkdir -p ${OBJ_DIR}
	${CC} ${CFLAGS} $^ -c -o ${OBJ_DIR}/$@
	
# Test
test:   plog
	@mkdir -p ${OBJ_DIR}
	@git submodule init
	@git submodule update
	${CC} -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread -c ${GTEST_DIR}/src/gtest-all.cc -o ${OBJ_DIR}/gtest-all.o
	ar -rv ${OBJ_DIR}/libgtest.a ${OBJ_DIR}/gtest-all.o
	${CC} -isystem ${GTEST_DIR}/include -pthread ${TEST_DIR}/* ${OBJ_DIR}/libgtest.a -o ${BIN_DIR}/test
	@${BIN_DIR}/test

# Install script, may need sudo permission
install:    plog
	cp ${BIN_DIR}/${BIN} /usr/local/bin/${BIN}

# Clean
clean:
	rm -rf ${BIN_DIR} ${OBJ_DIR}
