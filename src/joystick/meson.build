c_sources += files(
    'SDL_joystick.c',
    'SDL_gamecontroller.c',
)

subdir('dummy')
subdir('virtual')
subdir('steam')

if platform_is_android
    subdir('android')
elif platform_is_ios
    subdir('iphoneos')
elif platform_is_darwin
    subdir('darwin')
elif platform_is_linux
    subdir('linux')
elif platform_is_bsd
    subdir('bsd')
elif platform_is_emscripten
    subdir('emscripten')
elif platform_is_psp
    subdir('psp')
elif platform_is_windows
    subdir('windows')
endif
