# Test / demo programs

## oswrapper\_image
- test\_oswrapper\_image.c - demonstrates how to use oswrapper\_image to decode and retrieve information from an image file.
- test\_oswrapper\_image\_no\_crt.c - same as above, but without using the C runtime on Windows.

## oswrapper\_audio
- test\_oswrapper\_audio.c - demonstrates how to use oswrapper\_audio to decode an audio file to PCM data, and write the PCM data to another file.
- test\_oswrapper\_audio\_no\_crt.c - same as above, but without using the C runtime on Windows.
- demo\_oswrapper\_audio\_mac.c - decodes and plays an audio file with oswrapper\_audio, using macOS APIs for sound output.
- demo\_oswrapper\_audio\_miniaudio.c - decodes and plays an audio file with oswrapper\_audio, using miniaudio for sound output.
- demo\_oswrapper\_audio\_sokol\_audio.c - decodes and plays an audio file with oswrapper\_audio, using sokol\_audio for sound output.
- demo\_oswrapper\_audio\_sokol\_audio\_no\_crt.c - same as above, but without using the C runtime on Windows.