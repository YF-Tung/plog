BIN := plog
CC := g++
CFLAGS := --std=c++11 -Ofast -pthread

SRC_DIR := src
BIN_DIR := build
TEST_DIR := test
OBJ_DIR := obj
GTEST_DIR := googletest/googletest
INCLUDE := -I${SRC_DIR}

CPPS := $(notdir $(wildcard ${SRC_DIR}/*.cpp))
OBJS := $(patsubst %.cpp, %.o, ${CPPS})
OBJS_TEST := $(filter-out main.o, ${OBJS})  # Exclude main.o for testing


# Binaries
plog:    ${OBJS}
	@mkdir -p ${BIN_DIR}
	${CC} ${CFLAGS} $(addprefix ${OBJ_DIR}/, $^) -o ${BIN_DIR}/$@

# Objs
%.o:    ${SRC_DIR}/%.cpp
	@mkdir -p ${OBJ_DIR}
	${CC} ${CFLAGS} $^ -c -o ${OBJ_DIR}/$@

# Test
test:   clean plog libgtest.a
	@mkdir -p ${OBJ_DIR}
	${CC} ${CFLAGS} ${INCLUDE} -isystem ${GTEST_DIR}/include \
            ${TEST_DIR}/* ${OBJ_DIR}/libgtest.a \
            $(addprefix ${OBJ_DIR}/, ${OBJS_TEST}) -o ${BIN_DIR}/test
	@${BIN_DIR}/test

# Third party googletest
libgtest.a:
	@git submodule init
	@git submodule update
	${CC} -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread -c ${GTEST_DIR}/src/gtest-all.cc -o ${OBJ_DIR}/gtest-all.o
	ar -rv ${OBJ_DIR}/libgtest.a ${OBJ_DIR}/gtest-all.o


# Install script, may need sudo permission
install:    plog
	cp ${BIN_DIR}/${BIN} /usr/local/bin/${BIN}

# Clean
clean:
	rm -rf ${BIN_DIR} ${OBJ_DIR}
