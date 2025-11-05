cmake_minimum_required(VERSION 3.10)

project(plySpecial)

FILE(GLOB FILES_C "*.cpp" "*.hpp")
FILE(GLOB FILES_H "*.h" "*.inl")

add_library(${PROJECT_NAME}
${FILES_C} 
${FILES_H}
)