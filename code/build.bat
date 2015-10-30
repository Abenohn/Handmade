@echo off

mkdir ..\..\build\handmade
pushd ..\..\build\handmade
cl -FC -Zi ..\..\handmade\code\win32_handmade.cpp /link user32.lib Gdi32.lib 
popd