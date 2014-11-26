# Spectrogram Design

## File Structure

* `src`
    * `audio`
        * `AudioSource.hpp`: AudioSource abstract base class
        * `PulseAudioSource.cpp/hpp`: PulseAudio Source
        * `WaveAudioSource.cpp/hpp`: WAV File Soruce
    * `dft`
        * `RealDft.cpp/hpp`: Real DFT (FFTW wrapper)
    * `spectrogram`
        * `SpectrumRendeer.cpp/hpp`: DFT to pixels renderer
    * `image`
        * `ImageSink.hpp`: ImageSink abstract base class
        * `MagickImageSink.cpp/hpp`: GraphicsMagick Sink
        * `Orientation.hpp`: Orientation enumeration
    * `main`:
        * `AudioThread.cpp/hpp`: Audio input thread
        * `SpectrogramThread.cpp/hpp`: DFT and spectrum rendering thread
        * `InterfaceThread.cpp/hpp`: SDL interface thread
        * `Configuration.hpp`: Default options and limits
        * `ThreadSafeQueue.hpp`: Thread-safe queue helper class
        * `ThreadSafeResource.hpp`: Thread-safe resource locker class
        * `main.cpp`: Entry point and options parsing

## Classes

AudioSource

```
    owns audio device

    input source -> output samples

    get         sample rate
```

RealDft

```
    owns fftw plan and buffers

    input samples -> windowed samples -> output dft

    get/set     size, window function
```

SpectrumRenderer

```
    input dft -> output pixel row

    get/set     magnitude min, magnitude max, magnitude log., color scheme
    get         pixel to hz lambda
```


## Threads

AudioThread

```
    input AudioSource -> output samplesQueue
    ref to read size

    while True:
        read audio samples from AudioSource
        push samples into samplesQueue
```

SpectrogramThread

```
    input SamplesQueue -> output pixelsQueue
    ref to RealDft
    ref to SpectrumRenderer

    while True:
        pop new samples from samplesQueue
        shift new samples into sample buffer
        run RealDft on sample buffer to produce dft
        run SpectrumRenderer on dft to produce pixels
        push pixels into pixelsQueue
```

InterfaceThread

```
    input pixelsQueue -> output SDL
    ref to AudioSource (for configuration)
    ref to RealDft (for configuration)
    ref to SpectrumRenderer (for configuration)
    ref to read size (for control)

    while True:
        check and handle SDL events
        pop new pixels from pixelsQueue
        shift new pixels into pixel buffer
        draw pixel buffer to SDL
        draw settings info
```

