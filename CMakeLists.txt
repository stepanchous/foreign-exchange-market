CMAKE_MINIMUM_REQUIRED(VERSION 3.21)
PROJECT(test_market)

FIND_PACKAGE(Boost 1.40 COMPONENTS system REQUIRED)
FIND_PACKAGE(Threads REQUIRED)
FIND_PACKAGE(SQLite3 REQUIRED)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${SQLite3_INCLUDE_DIRS})

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CPP_COMPILER clang++)
SET(CMAKE_BUILD_TYPE Release)

ADD_EXECUTABLE(server.out ./src/server_main.cpp
               ./src/serializer.cpp ./src/serializer.h 
               ./src/server.cpp ./src/server.h 
               ./src/session.cpp ./src/session.h 
               ./src/market.cpp ./src/market.h 
               ./src/offer.cpp ./src/offer.h 
               ./src/deal.cpp ./src/deal.h 
               ./src/user_data.cpp ./src/user_data.h
               ./src/db_manager.cpp ./src/db_manager.h
               ./src/logger.cpp ./src/logger.h
               ./src/common.h ./src/json.h)
TARGET_LINK_LIBRARIES(server.out PRIVATE Threads::Threads ${Boost_LIBRARIES} ${SQLite3_LIBRARIES})

ADD_EXECUTABLE(client.out ./src/client_main.cpp 
               ./src/client.cpp ./src/client.h 
               ./src/common.h ./src/json.h
               ./src/request_handler.h ./src/request_handler.cpp)
TARGET_LINK_LIBRARIES(client.out PRIVATE Threads::Threads ${Boost_LIBRARIES})
