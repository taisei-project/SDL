/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

// Adapted from: https://github.com/devkitPro/SDL/blob/1f58ccf87bf3336e7418abcf1eea48940a63fe6d/src/audio/switch/SDL_switchaudio.c

#include "SDL_internal.h"

#if SDL_AUDIO_DRIVER_SWITCH

#include <malloc.h>
#include <string.h>

#include "SDL_switchaudio.h"

static bool SWITCHAUDIO_OpenDevice(SDL_AudioDevice *device)
{
    device->hidden = (struct SDL_PrivateAudioData *)SDL_malloc(sizeof(*device->hidden));
    if (device->hidden == NULL) {
        return SDL_OutOfMemory();
    }
    SDL_zerop(device->hidden);

    Result res = audoutInitialize();
    if (R_FAILED(res)) {
        SDL_free(device->hidden);
        device->hidden = NULL;
        return SDL_SetError("audoutInitialize failed (0x%x)", res);
    }

    PcmFormat fmt = audoutGetPcmFormat();

    switch (fmt) {
    case PcmFormat_Int8:
        device->spec.format = SDL_AUDIO_S8;
        break;
    case PcmFormat_Int16:
        device->spec.format = SDL_AUDIO_S16;
        break;
    case PcmFormat_Int32:
        device->spec.format = SDL_AUDIO_S32;
        break;
    case PcmFormat_Float:
        device->spec.format = SDL_AUDIO_F32;
        break;
    default:
        SDL_free(device->hidden);
        device->hidden = NULL;
        return SDL_SetError("audoutGetPcmFormat returned unsupported sample format (0x%x)", (int)fmt);
    }

    SDL_UpdatedAudioDeviceFormat(device);

    int aligned_size = (device->buffer_size + 0xfff) & ~0xfff;
    int mixlen = aligned_size * NUM_BUFFERS;

    device->hidden->rawbuf = memalign(0x1000, mixlen);
    if (device->hidden->rawbuf == NULL) {
        SDL_free(device->hidden);
        device->hidden = NULL;
        return SDL_SetError("Couldn't allocate mixing buffer");
    }

    SDL_memset(device->hidden->rawbuf, 0, mixlen);
    for (int i = 0; i < NUM_BUFFERS; i++) {
        device->hidden->out_buffers[i] = &device->hidden->rawbuf[i * aligned_size];
        device->hidden->buffer[i].next = NULL;
        device->hidden->buffer[i].buffer = device->hidden->out_buffers[i];
        device->hidden->buffer[i].buffer_size = aligned_size;
        device->hidden->buffer[i].data_size = device->buffer_size;
        device->hidden->buffer[i].data_offset = 0;
    }

    device->hidden->cur_buffer = device->hidden->next_buffer;
    device->hidden->next_buffer = (device->hidden->next_buffer + 1) % NUM_BUFFERS;

    res = audoutAppendAudioOutBuffer(&device->hidden->buffer[device->hidden->cur_buffer]);
    if (R_FAILED(res)) {
        free(device->hidden->rawbuf);
        device->hidden->rawbuf = NULL;
        SDL_free(device->hidden);
        device->hidden = NULL;
        return SDL_SetError("audoutAppendAudioOutBuffer failed (0x%x)", res);
    }

    res = audoutStartAudioOut();
    if (R_FAILED(res)) {
        free(device->hidden->rawbuf);
        device->hidden->rawbuf = NULL;
        return SDL_SetError("audoutStartAudioOut failed (0x%x)", res);
    }

    return true;
}

static bool SWITCHAUDIO_PlayDevice(SDL_AudioDevice *device, const Uint8 *data, int len)
{
    /* paranoia */
    SDL_assert(data == &device->hidden->buffer[device->hidden->cur_buffer].buffer);
    SDL_assert(len == device->buffer_size);

    device->hidden->cur_buffer = device->hidden->next_buffer;
    device->hidden->next_buffer = (device->hidden->next_buffer + 1) % NUM_BUFFERS;

    Result res = audoutAppendAudioOutBuffer(&device->hidden->buffer[device->hidden->cur_buffer]);
    if (R_FAILED(res)) {
        return SDL_SetError("audoutAppendAudioOutBuffer failed (0x%x)", res);
    }

    return true;
}

static bool SWITCHAUDIO_WaitDevice(SDL_AudioDevice *device)
{
    Result res = audoutWaitPlayFinish(&device->hidden->released_out_buffer, &device->hidden->released_out_count, UINT64_MAX);

    if (R_FAILED(res)) {
        return SDL_SetError("audoutWaitPlayFinish failed (0x%x)", res);
    }

    return true;
}

static Uint8 *SWITCHAUDIO_GetDeviceBuf(SDL_AudioDevice *device, int *buffer_size)
{
    return device->hidden->out_buffers[device->hidden->next_buffer];
}

static void SWITCHAUDIO_CloseDevice(SDL_AudioDevice *device)
{
    audoutStopAudioOut();
    audoutExit();

    if (device->hidden->rawbuf) {
        free(device->hidden->rawbuf);
        device->hidden->rawbuf = NULL;
    }

    SDL_free(device->hidden);
    device->hidden = NULL;
}

static bool SWITCHAUDIO_Init(SDL_AudioDriverImpl *impl)
{
    impl->OpenDevice = SWITCHAUDIO_OpenDevice;
    impl->PlayDevice = SWITCHAUDIO_PlayDevice;
    impl->WaitDevice = SWITCHAUDIO_WaitDevice;
    impl->GetDeviceBuf = SWITCHAUDIO_GetDeviceBuf;
    impl->CloseDevice = SWITCHAUDIO_CloseDevice;

    impl->OnlyHasDefaultPlaybackDevice = true;

    return 1;
}

// clang-format off
AudioBootStrap SWITCHAUDIO_bootstrap = {
    .name         = "switch",
    .desc         = "Nintendo Switch audio driver",
    .init         = SWITCHAUDIO_Init,
    .demand_only  = false,
    .is_preferred = true,
};
// clang-format on

#endif /* SDL_AUDIO_DRIVER_SWITCH */
