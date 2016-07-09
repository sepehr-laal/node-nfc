const fs = require('fs');
const path = require('path');

/*function cp(from, to){
  fs.copy(from, to, function (err) {
    if (err) return console.error(err)
    console.log("success!")
  });
}*/

function cp(from, to){
  var write = fs.createWriteStream(to);

  write.on('open',() => {
    fs.createReadStream(from)
      .pipe(write);
  });
}
cp(path.join(__dirname, 'bin/win/x86/libusb0_x86.dll'),path.join(__dirname, 'build/Release/libusb0.dll'));
cp(path.join(__dirname, 'bin/win/x86/libusb0_x86.dll'),path.join(__dirname, 'build/Release/libusb0_x86.dll'));
cp(path.join(__dirname, 'bin/win/x86/libnfc.dll'),path.join(__dirname, 'build/Release/libnfc.dll'));
