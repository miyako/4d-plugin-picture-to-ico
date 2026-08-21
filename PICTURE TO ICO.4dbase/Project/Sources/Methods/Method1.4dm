//%attributes = {}
$path:=Get 4D folder:C485(Current resources folder:K5:16)+"4D.png"

READ PICTURE FILE:C678($path; $icon)

PICTURE TO ICO($icon; $ico)

BLOB TO DOCUMENT:C526(System folder:C487(Desktop:K41:16)+"test.ico"; $ico)

