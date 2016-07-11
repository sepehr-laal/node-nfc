{
  "targets": [
    {
      "target_name": "nfc",
      "sources": [
        "src/nfc_module.cc",
		"src/nfc_wrapper.hh",
        "src/nfc_wrapper.cc",
        "src/nfc_auto_initializer.hh",
        "src/nfc_auto_initializer.cc"
      ],
      "libraries": [
        "../bin/win/x86/libnfc.lib"
      ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
		"include"
      ],
    }
  ]
}