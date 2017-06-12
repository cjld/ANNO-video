set -e

sudo apt-get install libfftw3-dev
echo "Build FPMLib"
mkdir -p ./build/FPMLib
cd build/FPMLib
qmake ../../FPMLib
make

cd ../..
echo "Build jumpcut"
mkdir -p ./build/jumpcut
cd build/jumpcut
qmake ../../jumpcut
make
