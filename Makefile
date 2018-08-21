BIN_DIR := build/
BIN := plog

default:    plog.cpp
	mkdir -p ${BIN_DIR}
	g++ --std=c++11 -Ofast $^ -o ${BIN_DIR}${BIN}
install:    default
	cp ${BIN_DIR}${BIN} /usr/local/bin/${BIN}
