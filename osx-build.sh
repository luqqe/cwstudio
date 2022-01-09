make clean
rm -rf OSX/*
rmdir OSX
mkdir -p OSX/cwwx.app/Contents/MacOS
mkdir -p OSX/cwwx.app/Contents/Resources
# mkdir -p OSX/cwcurses.app/Contents/MacOS
# mkdir -p OSX/cwcurses.app/Contents/Resources
make clean
./configure --with-wx-config=/Users/luke/wxWidgets-3.1.5/dist/wx-config
make

rm OSX/Applications
ln -s /Applications OSX/Applications

strip src/cwwx
strip src/cwcurses

cp src/cwwx OSX/cwwx.app/Contents/MacOS
cp src/cwwx.icns OSX/cwwx.app/Contents/Resources
cat > OSX/cwwx.app/Contents/Info.plist <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleGetInfoString</key>
  <string>CWWX</string>
  <key>CFBundleExecutable</key>
  <string>cwwx</string>
  <key>CFBundleIdentifier</key>
  <string>net.sourceforge.cwstudio</string>
  <key>CFBundleName</key>
  <string>cwwx</string>
  <key>CFBundleIconFile</key>
  <string>cwwx.icns</string>
  <key>CFBundleShortVersionString</key>
  <string>0.9.7</string>
  <key>CFBundleInfoDictionaryVersion</key>
  <string>0.9.7</string>
  <key>CFBundlePackageType</key>
  <string>APPL</string>
  <key>IFMajorVersion</key>
  <integer>9</integer>
  <key>IFMinorVersion</key>
  <integer>7</integer>
</dict>
</plist>
EOF

# cp src/cwcurses OSX/cwcurses.app/Contents/MacOS
# cp src/cwcurses.icns OSX/cwcurses.app/Contents/Resources
# cat > OSX/cwcurses.app/Contents/Info.plist <<EOF
# <?xml version="1.0" encoding="UTF-8"?>
# <!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
# <plist version="1.0">
# <dict>
#   <key>CFBundleGetInfoString</key>
#   <string>CWCurses</string>
#   <key>CFBundleExecutable</key>
#   <string>cwcurses</string>
#   <key>CFBundleIdentifier</key>
#   <string>net.sourceforge.cwstudio</string>
#   <key>CFBundleName</key>
#   <string>cwcurses</string>
#   <key>CFBundleIconFile</key>
#   <string>cwcurses.icns</string>
#   <key>CFBundleShortVersionString</key>
#   <string>0.9.7</string>
#   <key>CFBundleInfoDictionaryVersion</key>
#   <string>0.9.7</string>
#   <key>CFBundlePackageType</key>
#   <string>APPL</string>
#   <key>IFMajorVersion</key>
#   <integer>9</integer>
#   <key>IFMinorVersion</key>
#   <integer>7</integer>
# </dict>
# </plist>
# EOF
# 
rm tmp.dmg CWStudio.dmg

hdiutil create tmp.dmg -ov -volname "CWStudio" -fs HFS+ -srcfolder "OSX/" 
hdiutil convert tmp.dmg -format UDZO -o CWStudio.dmg
