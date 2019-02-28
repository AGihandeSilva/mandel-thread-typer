if "%1" == "clean" (
del CMakeCache.txt
)

"C:\Program Files\CMake\bin\cmake" .. -G"Visual Studio 15 2017 Win64"
