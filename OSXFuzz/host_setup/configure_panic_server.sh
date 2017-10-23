mkdir /PanicDumps
chown root:wheel /PanicDumps
chmod 1777 /PanicDumps
sudo launchctl load -w /System/Library/LaunchDaemons/com.apple.kdumpd.plist
