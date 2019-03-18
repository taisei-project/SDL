
if platform_is_unixlike
    subdir('unix')
endif

if platform_is_android
    subdir('android')
elif platform_is_linux
    subdir('linux')
elif platform_is_openbsd
    subdir('openbsd')
elif platform_is_freebsd
    subdir('freebsd')
elif platform_is_windows
    subdir('windows')
    if platform_is_winrt
        subdir('winrt')
    endif
elif platform_is_os2
    subdir('os2')
endif
