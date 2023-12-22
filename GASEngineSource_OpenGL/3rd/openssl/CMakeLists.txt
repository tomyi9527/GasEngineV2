project(openssl)
cmake_minimum_required(VERSION 2.8.0)

set(OPENSSL_DIR "openssl")

add_subdirectory(${OPENSSL_DIR})

# 可用的项目: ssl crypto