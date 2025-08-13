rm -rf out/build/*
mkdir -p out/build/
cmake -S . -B out/build/
cmake --build out/build
sudo cmake --install out/build --prefix /usr/local
