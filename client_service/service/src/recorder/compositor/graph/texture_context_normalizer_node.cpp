#include "recorder/compositor/graph/texture_context_normalizer_node.h"
#include "shared/log/log.h"

namespace service::recorder::compositor::graph {

TextureContextNormalizerNode::TextureContextNormalizerNode(service::renderer::D3d11SharedContext* context):
    _context(context)
{
    _newImage = std::make_unique<service::recorder::image::D3dImage>(_context);
    _tmpImage = std::make_unique<service::recorder::image::Image>();
}

void TextureContextNormalizerNode::receiveTexture(service::renderer::D3d11SharedContext* imageContext, ID3D11Texture2D* image, size_t numFrames, DXGI_MODE_ROTATION rotation) {
    // Check if we need to initialize our texture based on width/height.
    D3D11_TEXTURE2D_DESC desc;
    image->GetDesc(&desc);

    if (desc.Height != _newImage->height() || desc.Width != _newImage->width()) {
        _newImage->initializeImage(desc.Width, desc.Height, true);
    }

    // In the case that one DirectX context is on the CPU and the other is on the GPU then we need to do a
    // more complicated transfer.
    //      - In the case of GPU -> CPU, then we need to first transfer the texture into one of our own manual CPU image
    //        representations and load from that since the typical CopyResources tuff won't work.
    //      - In the case of CPU -> GPU, we error out because that shouldn't be a scenario we face currently. :)
    if (imageContext->deviceClass() == _context->deviceClass()) {
        // Note that in addition to changing the context the texture is on by copying it to a new texture,
        // we'll also remove the rotation in the input texture (if any).
        _newImage->copyFromSharedGpu(imageContext, image, rotation);
    } else if (_context->deviceClass() == service::renderer::D3d11Device::CPU) {
        if (desc.Height != _tmpImage->height() || desc.Width != _tmpImage->width()) {
            _tmpImage->initializeImage(desc.Width, desc.Height);
        }

        // It's theoretically possible to support rotations but that seems like a lot of work I don't want to do.
        assert(rotation == DXGI_MODE_ROTATION_IDENTITY);

        auto immediate = imageContext->immediateContext();
        _tmpImage->loadFromD3d11TextureWithStaging(imageContext->device(), immediate.context(), image);
        _newImage->copyFromCpu(*_tmpImage);
    } else {
        LOG_ERROR("Trying to transfer texture from the CPU to the GPU...currently unsupported." << std::endl);
        return;
    }

    flowToNext(_context, _newImage->rawTexture(), numFrames, DXGI_MODE_ROTATION_IDENTITY);
}

}