$ErrorActionPreference = "Stop"
$main = Get-Content "src/main.c" -Raw
if ($main -notmatch "UI task allocation failed") {
    throw "missing UI task allocation diagnostic"
}
$gif = Get-Content "lvgl/src/extra/libs/gif/lv_gif.c" -Raw
if ($gif -notmatch "gifobj->timer == NULL") {
    throw "missing GIF timer allocation guard"
}
