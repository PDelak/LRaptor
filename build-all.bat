setlocal

mkdir build-msvc
pushd build-msvc
cmake -G "Visual Studio 15 2017 Win64" ..
popd
cd build-msvc
cmake --build . --config release
cd ..
red -r lr-editor-ui\lr-editor.red
copy lr-editor.exe build-msvc\bin\lr-editor.exe
del lr-editor.exe