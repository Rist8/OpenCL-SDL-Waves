BUILD_DIR=$(shell pwd)/build

all:
	cmake -B${BUILD_DIR} .
	${MAKE} -C ${BUILD_DIR}

clean:
	rm -rf ${BUILD_DIR}

cppcheck:
	cppcheck \
	--enable=all \