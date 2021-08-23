#PROXY_WASM_CPP_SDK=./third_party/proxy-wasm-cpp-sdk
PROXY_WASM_CPP_SDK=/sdk

CPP_CONTEXT_LIB = ${PROXY_WASM_CPP_SDK}/proxy_wasm_intrinsics.cc
ModSecurity = /work/third_party/ModSecurity/include
ModSecurity_LIBS = /work/third_party/ModSecurity/libs/libmodsecurity.a

all: waf.wasm
	chown ${uid}.${gid} $^

%.wasm %.wat: %.cc ${PROXY_WASM_CPP_SDK}/proxy_wasm_intrinsics.h ${PROXY_WASM_CPP_SDK}/proxy_wasm_enums.h ${PROXY_WASM_CPP_SDK}/proxy_wasm_externs.h ${PROXY_WASM_CPP_SDK}/proxy_wasm_api.h ${PROXY_WASM_CPP_SDK}/proxy_wasm_intrinsics.js ${CPP_CONTEXT_LIB}
	    em++ --no-entry -s EXPORTED_FUNCTIONS=['_malloc'] --std=c++17 -O3 -flto -DPROXY_WASM_PROTOBUF_LITE=1 -I${ModSecurity} -I${PROXY_WASM_CPP_SDK} -I/usr/local/include --js-library ${PROXY_WASM_CPP_SDK}/proxy_wasm_intrinsics.js $*.cc ${PROXY_WASM_CPP_SDK}/proxy_wasm_intrinsics_lite.pb.cc ${PROXY_WASM_CPP_SDK}/struct_lite.pb.cc ${CPP_CONTEXT_LIB} ${PROXY_WASM_CPP_SDK}/libprotobuf-lite.a ${ModSecurity_LIBS} -o $*.wasm

clean: 
	rm *.wasm
