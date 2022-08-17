c_sources += files(
    'SDL_dbus.c',
    'SDL_evdev.c',
    'SDL_evdev_capabilities.c',
    'SDL_evdev_kbd.c',
    'SDL_ibus.c',
    'SDL_ime.c',
    'SDL_threadprio.c',
    'SDL_udev.c',
)

if cdata.get('HAVE_FCITX', 0) != 0
    c_sources += files('SDL_fcitx.c')
endif

