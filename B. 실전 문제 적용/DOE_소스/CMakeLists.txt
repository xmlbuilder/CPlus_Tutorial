cmake_minimum_required(VERSION 3.30)
project(DOE)

set(CMAKE_CXX_STANDARD 20)

include_directories(F:/library/eigen)

add_executable(DOE main.cpp
        Anom_Utils.h
        ResponseSurface.hpp
        doe_anom_response.hpp
        orthogonal_array.hpp
        response_surface_quadratic.hpp
        doe_full_analysis.hpp)
