#define cimg_use_png

#include <iostream>
#include <cstdint>
#include <filesystem>
#include <string>
#include <exception>
#include <CImg.h>
#include <gif_lib.h>

namespace fs = std::filesystem;
using namespace std::string_literals;
using namespace cimg_library;

constexpr auto DEFAULT_RESULT_DIR = "./result_per_frame";

class GifSplitter {
    GifFileType* gifFile = nullptr;
    int errorCode = 0;

    void load_gif(std::string const& gifPath) {
        gifFile = DGifOpenFileName(gifPath.c_str(), &errorCode);
        if (gifFile == nullptr) {
            throw std::runtime_error{ "Can't open gif file: "s + gifPath + ", error code is "s + std::to_string(errorCode) };
        }

        if (DGifSlurp(gifFile) != GIF_OK) {
            throw std::runtime_error{ "Can't read gif file: "s + gifPath };
        }
    }

    void create_result_dir() {
        if (!fs::exists(DEFAULT_RESULT_DIR)) {
            fs::create_directory(DEFAULT_RESULT_DIR);
        }
    }
public:
    explicit GifSplitter() {}

    ~GifSplitter() {
        if (gifFile != nullptr) {
            DGifCloseFile(gifFile, nullptr);
        }
    }

    void split_gif(std::string const& gifPath) {
        load_gif(gifPath);
        create_result_dir();

        fs::path p{ gifPath };

        for (int i = 0; i < gifFile->ImageCount; ++i) {
            GifImageDesc imageDesc = gifFile->SavedImages[i].ImageDesc;
            ColorMapObject* colorMap = (imageDesc.ColorMap ? imageDesc.ColorMap : gifFile->SColorMap);

            CImg<uint8_t> image(imageDesc.Width, imageDesc.Height, 1, 3);

            // copy pixel data
            for (int y = 0; y < imageDesc.Height; ++y) {
                for (int x = 0; x < imageDesc.Width; ++x) {
                    int index = imageDesc.Width * y + x;
                    GifByteType colorIndex = gifFile->SavedImages[i].RasterBits[index];
                    GifColorType color = colorMap->Colors[colorIndex];
                    image(x, y, 0) = color.Red;
                    image(x, y, 1) = color.Green;
                    image(x, y, 2) = color.Blue;
                }
            }

            // save frame as *.png file
            fs::path output_filename = DEFAULT_RESULT_DIR / fs::path{ (p.stem().string() + "_"s + std::to_string(i) + ".png"s)};
            std::cout << output_filename.string() << "\n";
            image.save_png(output_filename.string().c_str());
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " mygif.gif\n";
        return 1;
    }

    GifSplitter splitter;
    splitter.split_gif(argv[1]);
    return 0;
}
