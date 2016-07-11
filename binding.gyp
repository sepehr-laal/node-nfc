{
  "targets": [
    {
      "target_name": "nfc",
      "sources": [
        "src/nfc_module.cc",
		"include/nfc_wrapper.hh",
        "src/nfc_wrapper.cc",
        "include/nfc_auto_initializer.hh",
        "src/nfc_auto_initializer.cc",
        "include/nfc_card.hh",
        "src/nfc_card.cc",
        "include/nfc_read_worker.hh",
        "src/nfc_read_worker.cc"
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