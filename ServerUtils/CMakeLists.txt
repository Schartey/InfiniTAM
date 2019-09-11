##############################
# CMakeLists.txt for ServerUtils #
##############################

###########################
# Specify the target name #
###########################

SET(targetname ServerUtils)

################################
# Specify the libraries to use #
################################

INCLUDE(${PROJECT_SOURCE_DIR}/cmake/UseCUDA.cmake)

#############################
# Specify the project files #
#############################

SET(sources
TCPServer.cpp
TCPImageServer.cpp
)

SET(headers
TCPServer.h
TCPImageServer.h
)

#############################
# Specify the source groups #
#############################

SOURCE_GROUP("" FILES ${sources} ${headers})

##########################################
# Specify the target and where to put it #
##########################################

INCLUDE(${PROJECT_SOURCE_DIR}/cmake/SetCUDALibTarget.cmake)

if(WIN32)
  target_link_libraries(${targetname} wsock32 ws2_32)
endif()
