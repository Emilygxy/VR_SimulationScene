cmake_minimum_required(VERSION 3.8)
project(Interaction3DOF)

# 设置输出目录（确保所有配置都输出到 bin/ 目录）
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
function(get_config_output_path OUT_VAR)
    # 动态生成路径：自动适配单/多配置生成器
    set(config_path "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>")
    set(${OUT_VAR} "${config_path}" PARENT_SCOPE)
endfunction()

# 调用函数获取当前路径
get_config_output_path(CURRENT_CONFIG_OUTPUT_DIR)

set(GLFW_GLAD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL/include)
set(GLFW_LIB ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL/lib/glfw3.lib)

#收集文件
file(GLOB SOURCES  
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
				   ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/*.cpp
				   ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL/src/*.c)
file(GLOB HEADERS 
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
				  ${CMAKE_CURRENT_SOURCE_DIR}/include/utils/*.h
                )

set(THIRD_PARTY_LIB 
	${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/glm/lib/glm.lib
	# more
	)

set(THIRD_PARTY_INCLUDE_DIR 
	${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/glm/
	# more
	)

#添加依赖的头文件
include_directories(
	${GLFW_GLAD_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${THIRD_PARTY_INCLUDE_DIR})

set(ALL_LIBS
	${GLFW_LIB}
	${THIRD_PARTY_LIB}
)

#生成可执行程序
add_executable(Interaction3DOF 
			${HEADERS}
			${SOURCES})  ## 

target_link_libraries(Interaction3DOF PUBLIC ${ALL_LIBS})
