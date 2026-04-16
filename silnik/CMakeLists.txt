cmake_minimum_required(VERSION 4.2)
project(silnik)

set(CMAKE_CXX_STANDARD 20)

add_executable(silnik main.cpp
        models.h
        supportFunctions.cpp
        supportFunctions.h
        dataReader.cpp
        dataReader.h
        timeLineBuilder.cpp
        timeLineBuilder.h
        dataWriter.cpp
        dataWriter.h
        algorytmGNR1.cpp
        algorytmGNR1.h
        generateDay.cpp
)
target_link_options(silnik PRIVATE "-static")