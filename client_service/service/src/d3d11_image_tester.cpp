#include <boost/program_options.hpp>
#include <filesystem>
#include "recorder/image/image.h"
#include "recorder/image/d3d_image.h"
#include "shared/log/log.h"

namespace po = boost::program_options;
namespace fs = std::filesystem;

int main(int argc, char** argv) {
    po::options_description desc("Options");
    desc.add_options()
        ("input", po::value<std::string>()->required(), "Input image to load into the D3D Image.")
        ("mode", po::value<std::string>()->required(), "CPU or GPU")
        ("width", po::value<unsigned int>()->required(), "Width of output")
        ("height", po::value<unsigned int>()->required(), "Height of output")
        ("output", po::value<std::string>()->required(), "Output image file path for the final image.");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

    const auto outputPath = fs::path(vm["output"].as<std::string>());

    // Load image from file.
    service::recorder::image::Image image;
    image.loadFromFile(fs::path(vm["input"].as<std::string>()));

    const auto mode = vm["mode"].as<std::string>();

    const auto width = vm["width"].as<unsigned int>();
    const auto height = vm["height"].as<unsigned int>();

    service::renderer::D3d11SharedContext shared;

    service::recorder::image::D3dImage outputImage(&shared);

    // Setup D3D11 device/context for use in the GPU pipeline.
    ID3D11Texture2D* copyTexture = nullptr;

    if (mode == "CPU") {
        outputImage.initializeImage(width, height);
        outputImage.copyFromCpu(image);
    } else {
        outputImage.initializeImage(height, width);
        // Manually convert the image to a direct3D 11 texture.
        D3D11_SUBRESOURCE_DATA resource;
        resource.pSysMem = image.buffer();
        resource.SysMemPitch = image.numBytesPerRow();
        resource.SysMemSlicePitch = image.numBytes();

        D3D11_TEXTURE2D_DESC copyDesc = { 0 };
        copyDesc.Width = image.width();
        copyDesc.Height = image.height();
        copyDesc.MipLevels = 1;
        copyDesc.ArraySize = 1;
        copyDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        copyDesc.SampleDesc.Count = 1;
        copyDesc.Usage = D3D11_USAGE_DEFAULT;
        copyDesc.BindFlags = 0;
        copyDesc.CPUAccessFlags = 0;
        copyDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

        if (shared.device()->CreateTexture2D(&copyDesc, &resource, &copyTexture) != S_OK) {
            LOG_ERROR("Failed to create copy texture."<< std::endl);
            return 1;
        }

        outputImage.copyFromGpu(copyTexture, DXGI_MODE_ROTATION_ROTATE90);
    }

    outputImage.cpuImage().saveToFile(outputPath);

    if (copyTexture) {
        copyTexture->Release();
    }
    return 0;
}