call "C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Auxiliary/Build/vcvarsall.bat" x64

cmake -G "Visual Studio 17 2022" -H. -Bbuild -DHUNTER_ENABLED=ON -DHUNTER_ROOT="C:\hunter" -DCMAKE_BUILD_TYPE=Release -DHUNTER_CONFIGURATION_TYPES=Release -DUSE_BUNDLED_GTEST=OFF -DBUILD_LIB_TESTS=OFF -DBUILD_LIB_EXAMPLES=OFF -DBUILD_SHARED_LIBS=OFF
cmake --build build --config Release -j %NUMBER_OF_PROCESSORS%

