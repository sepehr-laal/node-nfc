{
  "build_files": [
    "../deps/win32/libusb0.dll",
    "../deps/win32/libnfc.dll",
    "../deps/win32/libusb0_x86.dll"
  ],
  "targets": [
    {
      "target_name": "nfc",
      "sources": [
        "src/nfc.cc"
      ],
      "libraries": [
        "../bin/win/x86/libnfc.lib"
      ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ],
      "conditions": [
        [
          "OS==\"win\"",
          {
            "include_dirs": [
              "./deps/win32"
            ]
          }
        ]
      ]
    }
  ]
}