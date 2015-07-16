del .\compiled\PtokaX.exe
del .\compiled\PtokaX-gui.exe
del .\compiled\PtokaX.pdb
del .\compiled\PtokaX-gui.pdb

call "%VS100COMNTOOLS%\..\..\VC\bin\vcvars32.bat"
"%VS100COMNTOOLS%..\ide\devenv" PtokaX-gui.sln /Rebuild "Release|Win32"

call "%VS100COMNTOOLS%\..\..\VC\bin\vcvars32.bat"
"%VS100COMNTOOLS%..\ide\devenv" PtokaX.sln /Rebuild "Release|Win32"

call src-gen-filename.bat -src
7z.exe a -r -t7z -m0=lzma -mx=9 -mfb=512 -md=1024m -ms=on %FILE_NAME%.7z -i@src-include.txt -x@src-exclude.txt -x!./compiled/*

SYMUPLOAD 70D71CA7-8FDF-41C4-9AA1-4ADB30527A54 0.5.0.1 0 .\compiled\PtokaX-gui.pdb
SYMUPLOAD 70D71CA7-8FDF-41C4-9AA1-4ADB30527A54 0.5.0.1 0 .\compiled\PtokaX-gui.exe

SYMUPLOAD 11B2CC9B-B1E9-4894-9AE6-6C4CB3DFDECA 0.5.0.1 0 .\compiled\PtokaX.pdb
SYMUPLOAD 11B2CC9B-B1E9-4894-9AE6-6C4CB3DFDECA 0.5.0.1 0 .\compiled\PtokaX.exe
