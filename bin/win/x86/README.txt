This is simply a compilation of libnfc per instruction for Win32.
http://www.libnfc.org/

Following the README-Windows.txt with the following packages:

- Installing Windows SDK 7.1A (Windows 7 with .NET 4)
- libusb win32 1.2.6.0
- CMake 2.8.10.1
- libnfc 1.6.0 rc1

This package was hand-rolled, instead of leveraging the CMake and pkg-config.
That may come later.

The one modification made was already submitted and accepted to libnfc to build a proper DEBUG_OUTPUT spewing version dll (in the debug directory) in revision d0ea9fdd752a.

Running the tools:

- You must bring in a libusb dll, rename and copy the libusb0-x86.dll
- If you would like to see the debug output, replace the current 
  libnfc.dll with the one in the debug directory.

NOTE: Getting your USB reader still requires libusb driver installation.
      Recommend using zadig.exe to get that done.