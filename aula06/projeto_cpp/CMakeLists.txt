cmake_minimum_required(VERSION 3.10)
project(grpc_lab CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

# Encontra os pacotes de biblioteca
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# 1. Encontra os executáveis de compilação de forma explícita
#    Encontra o 'protoc'
find_program(PROTOC_EXECUTABLE protoc)
if(NOT PROTOC_EXECUTABLE)
    message(FATAL_ERROR "Compilador 'protoc' não encontrado. Verifique se o pacote 'protobuf-compiler' está instalado.")
endif()

#    Encontra o plugin do gRPC para C++ pelo nome correto no Ubuntu
find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin)
if(NOT GRPC_CPP_PLUGIN)
    message(FATAL_ERROR "Plugin 'grpc_cpp_plugin' não encontrado. Verifique se o pacote 'libgrpc-dev' ou 'protobuf-compiler-grpc' está instalado.")
endif()

# 2. Define os nomes dos arquivos que serão gerados a partir de notas.proto
set(PROTO_SRC_FILES ${CMAKE_CURRENT_BINARY_DIR}/notas.pb.cc)
set(PROTO_HDR_FILES ${CMAKE_CURRENT_BINARY_DIR}/notas.pb.h)
set(GRPC_SRC_FILES  ${CMAKE_CURRENT_BINARY_DIR}/notas.grpc.pb.cc)
set(GRPC_HDR_FILES  ${CMAKE_CURRENT_BINARY_DIR}/notas.grpc.pb.h)

# 3. Define o comando customizado para gerar os arquivos
add_custom_command(
    OUTPUT ${PROTO_SRC_FILES} ${PROTO_HDR_FILES} ${GRPC_SRC_FILES} ${GRPC_HDR_FILES}

    # Comando para gerar os arquivos .pb.cc/.h (mensagens)
    COMMAND ${PROTOC_EXECUTABLE}
        --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
        -I${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/notas.proto

    # Comando para gerar os arquivos .grpc.pb.cc/.h (serviços)
    COMMAND ${PROTOC_EXECUTABLE}
        --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
        --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN} # A sintaxe name=path ainda é a mais correta
        -I${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/notas.proto

    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/notas.proto
    COMMENT "Gerando código a partir de notas.proto"
)

# 4. Cria um alvo para garantir que a geração de código aconteça
add_custom_target(generate_grpc_files ALL DEPENDS
    ${PROTO_SRC_FILES} ${PROTO_HDR_FILES} ${GRPC_SRC_FILES} ${GRPC_HDR_FILES}
)

# --- Seção de Compilação dos Executáveis ---

# Adiciona o diretório dos cabeçalhos gerados ao include path
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Adiciona o executável do servidor
add_executable(servidor_cpp servidor.cpp ${PROTO_SRC_FILES} ${GRPC_SRC_FILES})
target_link_libraries(servidor_cpp PRIVATE protobuf::libprotobuf gRPC::grpc++)

# Adiciona o executável do cliente
add_executable(cliente_cpp cliente.cpp ${PROTO_SRC_FILES} ${GRPC_SRC_FILES})
target_link_libraries(cliente_cpp PRIVATE protobuf::libprotobuf gRPC::grpc++)