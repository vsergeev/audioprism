#include <thread>
#include <iostream>
#include <getopt.h>

#include "image/Orientation.hpp"
#include "audio/PulseAudioSource.hpp"
#include "dft/RealDft.hpp"
#include "spectrogram/SpectrumRenderer.hpp"

#include "audio/WaveAudioSource.hpp"
#include "image/MagickImageSink.hpp"

#include "ThreadSafeResource.hpp"
#include "ThreadSafeQueue.hpp"

#include "AudioThread.hpp"
#include "SpectrogramThread.hpp"
#include "InterfaceThread.hpp"
#include "Configuration.hpp"

using namespace Audio;
using namespace DFT;
using namespace Spectrogram;
using namespace Image;

namespace Configuration {
Settings InitialSettings;
Limits UserLimits;
}

using namespace Configuration;

void spectrogram_realtime() {
    PulseAudioSource audio(InitialSettings.audioSampleRate);
    RealDft dft(InitialSettings.dftSize, InitialSettings.dftWf);
    SpectrumRenderer spectrogram(InitialSettings.magnitudeMin, InitialSettings.magnitudeMax, InitialSettings.magnitudeLog, InitialSettings.colors);

    ThreadSafeResource<AudioSource> audioResource(audio);
    ThreadSafeResource<RealDft> dftResource(dft);
    ThreadSafeResource<SpectrumRenderer> spectrogramResource(spectrogram);
    ThreadSafeQueue<std::vector<double>> samplesQueue;
    ThreadSafeQueue<std::vector<uint32_t>> pixelsQueue;
    std::atomic<size_t> audioReadSize;
    std::atomic<bool> running;

    audioReadSize = InitialSettings.audioReadSize;
    running = true;

    InterfaceThread interfaceThread(pixelsQueue, audioResource, dftResource, spectrogramResource, audioReadSize, running, InitialSettings.width, InitialSettings.height, InitialSettings.orientation);
    std::thread audioThread(AudioThread, std::ref(audioResource), std::ref(samplesQueue), std::ref(audioReadSize), std::ref(running));
    std::thread spectrogramThread(SpectrogramThread, std::ref(samplesQueue), std::ref(pixelsQueue), std::ref(dftResource), std::ref(spectrogramResource), (InitialSettings.orientation == Orientation::Vertical) ? InitialSettings.width : InitialSettings.height, std::ref(running));

    interfaceThread.run();

    audioThread.join();
    spectrogramThread.join();
}

void spectrogram_audiofile(std::string audioPath, std::string imagePath) {
    WaveAudioSource audio(audioPath);
    RealDft dft(InitialSettings.dftSize, InitialSettings.dftWf);
    SpectrumRenderer spectrogram(InitialSettings.magnitudeMin, InitialSettings.magnitudeMax, InitialSettings.magnitudeLog, InitialSettings.colors);
    MagickImageSink image(imagePath, InitialSettings.width, InitialSettings.orientation);

    std::vector<double> newSamples(InitialSettings.audioReadSize);
    std::vector<double> samples(InitialSettings.dftSize);
    std::vector<std::complex<double>> dftSamples(InitialSettings.dftSize);
    std::vector<uint32_t> pixels(InitialSettings.width);

    while (true) {
        audio.read(newSamples);

        if (newSamples.size() == 0)
            break;

        /* Move down old samples */
        memmove(samples.data(), samples.data()+newSamples.size(), sizeof(double)*(samples.size()-newSamples.size()));
        /* Copy new samples */
        memcpy(samples.data()+(samples.size()-newSamples.size()), newSamples.data(), sizeof(double)*newSamples.size());

        dft.compute(dftSamples, samples);

        spectrogram.render(pixels, dftSamples);

        image.append(pixels);
    }

    image.write();
}

void print_usage(std::string progname) {
    std::cerr << "\
Interactive Usage: " << progname << " [options]\n\
 Audio File Usage: " << progname << " [options] <audio file input> <image file output>\n\
\n\
Interface Settings\n\
    -h,--help                   Help\n\
    --width <width>             Width of spectrogram (default 640)\n\
    --height <height>           Height of spectrogram (default 480)\n\
    --orientation <orientation> Orientation [horizontal, vertical]\n\
                                    (default vertical)\n\
\n\
Audio Settings\n\
    -r,--sample-rate <rate>     Audio input sample rate (default 24000)\n\
\n\
DFT Settings\n\
    --overlap <percentage>      Overlap percentage (default 50%)\n\
    --dft-size <size>           DFT Size, must be power of two (default 2048)\n\
    --window <window function>  Window Function [hanning, hamming, rectangular]\n\
                                    (default hanning)\n\
\n\
Spectrogram Settings\n\
    --magnitude-scale <scale>   Magnitude Scale [linear, logarithmic]\n\
                                    (default logarithmic)\n\
    --magnitude-min <value>     Magnitude Minimum (default 0.0)\n\
    --magnitude-max <value>     Magnitude Maximum (default 50.0)\n\
    --colors <color scheme>     Color Scheme [heat, blue, grayscale]\n\
                                    (default heat)\n\
\n\
Interactive Keyboard Control:\n\
    q           - Quit\n\
    h           - Hide settings information\n\
    c           - Cycle color scheme\n\
    w           - Cycle window function\n\
    l           - Toggle logarithmic/linear magnitude\n\
\n\
    -           - Decrease minimum magnitude\n\
    =           - Increase minimum magnitude\n\
\n\
    [           - Decrease maximum magnitude\n\
    ]           - Increase maximum magnitude\n\
\n\
    Left arrow  - Decrease DFT Size\n\
    Right arrow - Increase DFT Size\n\
\n\
    Down arrow  - Decrease overlap\n\
    Up arrow    - Increase overlap\n" << std::endl;
}

int main(int argc, char *argv[]) {
    unsigned int overlap = 50;

    static struct option long_options[] = {
        {"help",            no_argument,        0,  'h'},
        {"width",           required_argument,  0,  0},
        {"height",          required_argument,  0,  0},
        {"orientation",     required_argument,  0,  0},
        {"sample-rate",     required_argument,  0,  'r'},
        {"overlap",         required_argument,  0,  0},
        {"dft-size",        required_argument,  0,  0},
        {"window",          required_argument,  0,  0},
        {"magnitude-scale", required_argument,  0,  0},
        {"magnitude-min",   required_argument,  0,  0},
        {"magnitude-max",   required_argument,  0,  0},
        {"colors",          required_argument,  0,  0},
    };

    while (1) {
        int options_index;
        int c = getopt_long(argc, argv, "hr:", long_options, &options_index);

        if (c == -1) {
            break;
        } else if (c == 'h') {
            print_usage(argv[0]);
            return EXIT_FAILURE;
        } else if (c == 'r') {
            std::cout << "option r with _" << optarg << "_" << std::endl;
        } else if (c == 0) {
            std::string option_name = long_options[options_index].name;
            std::string option_arg = (long_options[options_index].has_arg == required_argument) ? optarg : "";

            if (option_name == "orientation") {
                if (option_arg == "horizontal") {
                    InitialSettings.orientation = Orientation::Horizontal;
                } else if (option_arg == "vertical") {
                    InitialSettings.orientation = Orientation::Vertical;
                } else {
                    std::cerr << "Invalid value for orientation.\n\n";
                    print_usage(argv[0]);
                    return EXIT_FAILURE;
                }
            } else if (option_name == "width") {
                try {
                    InitialSettings.width = std::stoul(option_arg);
                } catch (const std::invalid_argument &e) {
                    std::cerr << "Invalid value for width.\n\n";
                    print_usage(argv[0]);
                    return EXIT_FAILURE;
                }
            } else if (option_name == "height") {
                try {
                    InitialSettings.height = std::stoul(option_arg);
                } catch (const std::invalid_argument &e) {
                    std::cerr << "Invalid value for height.\n\n";
                    print_usage(argv[0]);
                    return EXIT_FAILURE;
                }
            } else if (option_name == "overlap") {
                try {
                    overlap = std::stoul(option_arg);
                } catch (const std::invalid_argument &e) {
                    std::cerr << "Invalid value for overlap.\n\n";
                    print_usage(argv[0]);
                    return EXIT_FAILURE;
                }
                if (overlap > 100) {
                    std::cerr << "Invalid value for overlap (must be >= 0 and <= 100).\n\n";
                    print_usage(argv[0]);
                    return EXIT_FAILURE;
                }
            } else if (option_name == "dft-size") {
                unsigned int dftSize;
                try {
                    dftSize = std::stoul(option_arg);
                } catch (const std::invalid_argument &e) {
                    std::cerr << "Invalid value for DFT size.\n\n";
                    print_usage(argv[0]);
                    return EXIT_FAILURE;
                }

                if ((dftSize & (dftSize - 1)) != 0 || dftSize < UserLimits.dftSizeMin || dftSize  > UserLimits.dftSizeMax) {
                    std::cerr << "Invalid value for DFT size (must be power of 2 and >= " << UserLimits.dftSizeMin << " and <= " << UserLimits.dftSizeMax << ").\n\n";
                    print_usage(argv[0]);
                    return EXIT_FAILURE;
                }

                InitialSettings.dftSize  = dftSize;
            } else if (option_name == "window") {
                if (option_arg == "hanning")
                    InitialSettings.dftWf = RealDft::WindowFunction::Hanning;
                else if (option_arg == "hamming")
                    InitialSettings.dftWf = RealDft::WindowFunction::Hamming;
                else if (option_arg == "rectangular")
                    InitialSettings.dftWf = RealDft::WindowFunction::Rectangular;
                else {
                    std::cerr << "Invalid window function.\n\n";
                    print_usage(argv[0]);
                    return EXIT_FAILURE;
                }
            } else if (option_name == "magnitude-scale") {
                if (option_arg == "logarithmic")
                    InitialSettings.magnitudeLog = true;
                else if (option_arg == "linear")
                    InitialSettings.magnitudeLog = false;
                else {
                    std::cerr << "Invalid magnitude scale.\n\n";
                    print_usage(argv[0]);
                    return EXIT_FAILURE;
                }
            } else if (option_name == "magnitude-min") {
                try {
                    InitialSettings.magnitudeMin = std::stod(option_arg);
                } catch (const std::invalid_argument &e) {
                    std::cerr << "Invalid magnitude minimum.\n\n";
                    print_usage(argv[0]);
                    return EXIT_FAILURE;
                }
            } else if (option_name == "magntiude-max") {
                try {
                    InitialSettings.magnitudeMax = std::stod(option_arg);
                } catch (const std::invalid_argument &e) {
                    std::cerr << "Invalid magnitude maximum.\n\n";
                    print_usage(argv[0]);
                    return EXIT_FAILURE;
                }
            } else if (option_name == "colors") {
                if (option_arg == "logarithmic")
                    InitialSettings.magnitudeLog = true;
                else if (option_arg == "linear")
                    InitialSettings.magnitudeLog = false;
                else {
                    std::cerr << "Invalid magnitude scale.\n\n";
                    print_usage(argv[0]);
                    return EXIT_FAILURE;
                }
            }
        }
    }

    /* Validate magnitude min/max with magnitude scale mode */
    if (InitialSettings.magnitudeLog) {
        if (InitialSettings.magnitudeMin < UserLimits.magnitudeLogMin) {
            std::cout << "Invalid magnitude min (must be >= " << UserLimits.magnitudeLogMin << ").\n\n";
            print_usage(argv[0]);
            return EXIT_FAILURE;
        } else if (InitialSettings.magnitudeMax > UserLimits.magnitudeLogMax ){
            std::cout << "Invalid magnitude max (must be <= " << UserLimits.magnitudeLogMax << ").\n\n";
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    } else {
        if (InitialSettings.magnitudeMin < UserLimits.magnitudeLogMin) {
            std::cout << "Invalid magnitude min (must be >= " << UserLimits.magnitudeLinearMin << ").\n\n";
            print_usage(argv[0]);
            return EXIT_FAILURE;
        } else if (InitialSettings.magnitudeMax > UserLimits.magnitudeLogMax ){
            std::cout << "Invalid magnitude max (must be <= " << UserLimits.magnitudeLinearMax << ").\n\n";
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    /* Compute overlap with DFT Size */
    InitialSettings.audioReadSize = static_cast<unsigned int>((static_cast<float>(overlap)/100.0)*static_cast<float>(InitialSettings.dftSize));

    if ((argc - optind) > 0 && (argc - optind) != 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    /* Audio file mode */
    if ((argc - optind) == 2) {
        spectrogram_audiofile(std::string(argv[optind]), std::string(argv[optind+1]));

    /* Realtime mode */
    } else {
        spectrogram_realtime();
    }

    return 0;
}

