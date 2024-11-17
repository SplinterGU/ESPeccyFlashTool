cmake_minimum_required(VERSION 3.10)

# Definir el nombre del proyecto
project(ESPeccyFlashTool)

# Configurar las opciones de compilación específicas para el sistema operativo
if(WIN32)
    # Configuración para Windows
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
elseif(UNIX)
    # Configuración para Linux
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
endif()

# Buscar las bibliotecas necesarias
find_package(CURL REQUIRED)
#find_library(JANSSON_LIB jansson REQUIRED)

# Agregar el ejecutable
add_executable(especcy_flash_tool download_file.c esp32-detect.c main.c)

# Incluir directorios de cabecera necesarios
target_include_directories(especcy_flash_tool PRIVATE
    ${CURL_INCLUDE_DIRS}
)

# Enlazar las bibliotecas necesarias
target_link_libraries(especcy_flash_tool PRIVATE
    ${CURL_LIBRARIES}
    jansson
)
