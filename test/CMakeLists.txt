CMAKE_MINIMUM_REQUIRED(VERSION 3.21)
PROJECT(test_market)

FIND_PACKAGE(Catch2 3 REQUIRED)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

SET(CMAKE_CPP_COMPILER clang++)
SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_BUILD_TYPE Debug)
ADD_COMPILE_DEFINITIONS(TEST)

ADD_EXECUTABLE(tests.out test_market.cpp 
               ../src/market.cpp ../src/market.h 
               ../src/offer.cpp ../src/offer.h 
               ../src/user_data.cpp ../src/user_data.h 
               ../src/deal.cpp ../src/deal.h)

TARGET_LINK_LIBRARIES(tests.out PRIVATE Catch2::Catch2WithMain)
