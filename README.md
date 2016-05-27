# 4d-plugin-picture-to-ico
Create large ICO file from picture.

##Platform

| carbon | cocoa | win32 | win64 |
|:------:|:-----:|:---------:|:---------:|
|🆗|🆗|🆗|🆗|

File contains size 16, 32, 48, 64, 128 and 256.

If the passed picture is not PNG, it is internally converted by the plugin.

##Example

```
$path:=Get 4D folder(Current resources folder)+"4D.png"

READ PICTURE FILE($path;$icon)

PICTURE TO ICO ($icon;$ico)

BLOB TO DOCUMENT(System folder(Desktop)+"test.ico";$ico)
```
