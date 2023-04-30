# Test / demo programs

## oswrapper\_image
- test\_oswrapper\_image.c - demonstrates how to use oswrapper\_image to decode and retrieve information from an image file.
- test\_oswrapper\_image\_no\_crt.c - same as above, but without using the C runtime on Windows.

## oswrapper\_audio
- test\_oswrapper\_audio.c - demonstrates how to use oswrapper\_audio to decode an audio file to PCM data, and write the PCM data to another file.
- test\_oswrapper\_audio\_no\_crt.c - same as above, but without using the C runtime on Windows.
- test\_oswrapper\_audio\_enc.c - decodes an audio file with oswrapper\_audio, and encodes the PCM data to a variety of formats using oswrapper\_audio\_enc.
- test\_oswrapper\_audio\_enc\_mod.c - decodes a ProTracker MOD file with pocketmod, and encodes the PCM data to a variety of formats using oswrapper\_audio\_enc.
- test\_oswrapper\_audio\_mac\_encoder.c - decodes an audio file with oswrapper\_audio, and encodes the PCM data to M4A using macOS APIs.
- test\_oswrapper\_audio\_win\_encoder.c - decodes an audio file with oswrapper\_audio, and encodes the PCM data to WAV using Windows APIs.
- test\_oswrapper\_audio\_win\_encoder\_no\_crt.c - same as above, but without using the C runtime on Windows.
- demo\_oswrapper\_audio\_mac.c - decodes and plays an audio file with oswrapper\_audio, using macOS APIs for sound output.
- demo\_oswrapper\_audio\_miniaudio.c - decodes and plays an audio file with oswrapper\_audio, using miniaudio for sound output.
- demo\_oswrapper\_audio\_sokol\_audio.c - decodes and plays an audio file with oswrapper\_audio, using sokol\_audio for sound output.
- demo\_oswrapper\_audio\_sokol\_audio\_no\_crt.c - same as above, but without using the C runtime on Windows.

## Example file credits

- [ELYSIUM.MOD](https://modarchive.org/index.php?request=view_by_moduleid&query=40475) - Elysium, by Jester. Licensed under the [Attribution Non-commercial Share Alike license](https://creativecommons.org/licenses/by-nc-sa/4.0/).
