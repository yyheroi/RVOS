cmake -B build-x86_64 -G Ninja -DCMAKE_BUILD_TYPE=Release

cmake --build build-x86_64 --clean-first -v