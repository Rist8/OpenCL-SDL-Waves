BUILD_DIR=$(shell pwd)/build

all:
	cmake -B${BUILD_DIR} .
	${MAKE} -C ${BUILD_DIR}
	cp src.cl ${BUILD_DIR}/src.cl

clean:
	rm -rf ${BUILD_DIR}

cppcheck:
	cppcheck .\
