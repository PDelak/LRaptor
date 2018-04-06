setlocal

mkdir build-make
pushd build-make
cmake -G "MinGW Makefiles" ..
popd 
cd build-make
cmake --build . --config release
cd ..
red -r lr-editor-ui\lr-editor.red
copy lr-editor.exe build-make\bin\lr-editor.exe
del lr-editor.exe