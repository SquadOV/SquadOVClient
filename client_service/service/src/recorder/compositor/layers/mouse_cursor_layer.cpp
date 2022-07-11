#include "recorder/compositor/layers/mouse_cursor_layer.h"
#include "shared/log/log.h"
#include "shared/errors/error.h"
#include <wil/com.h>

namespace service::recorder::compositor::layers {

MouseCursorLayer::MouseCursorLayer(HWND parentWindow):
    _parentWindow(parentWindow)
{
    
}

void MouseCursorLayer::updateAt(const service::recorder::encoder::AVSyncClock::time_point& tp, service::renderer::D3d11Renderer* renderer, ID3D11DeviceContext* context) {
    // Make sure we're invisible by default.
    _model->setVisibility(false);

    CURSORINFO info = { 0 };
    info.cbSize = sizeof(CURSORINFO);

    if (!GetCursorInfo(&info)) {
        LOG_WARNING("Failed to get cursor info: " << shared::errors::getWin32ErrorAsString() << std::endl);
        return;
    }

    const bool isVisible = _enabled && (info.flags & CURSOR_SHOWING);
    if (!isVisible) {
        return;
    }

    // The cursor comes in screen position so we convert it into client position
    // (i.e. relative to the window in question).
    POINT cursorPosition = { info.ptScreenPos.x, info.ptScreenPos. y};
    if (!ScreenToClient(_parentWindow, &cursorPosition)) {
        return;
    }

    // Check whether or not the cursor is out of bounds so we can easily skip.
    RECT windowRes;
    if (!GetClientRect(_parentWindow, &windowRes)) {
        return;
    }

    if (cursorPosition.x < windowRes.left || cursorPosition.x > windowRes.right ||
        cursorPosition.y < windowRes.top || cursorPosition.y > windowRes.bottom) {
        return;
    }

    // It doesn't really matter if this function call fails. We only use it to check the hotspot
    // and if that's 0 whatever.
    ICONINFO iconInfo = { 0 };
    GetIconInfo(info.hCursor, &iconInfo);

    // Now we need to get the width/height of the icon by grabbing the bitmap.
    D3D11_TEXTURE2D_DESC desc = { 0 };
    desc.Width = 0;
    desc.Height = 0;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = D3D11_RESOURCE_MISC_GDI_COMPATIBLE;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = 1;
    desc.ArraySize = 1;

    BITMAP bitmap = { 0 };
    if (iconInfo.hbmColor) {
        if (GetObject(iconInfo.hbmColor, sizeof(bitmap), &bitmap) > 0) {
            desc.Width = bitmap.bmWidth;
            desc.Height = bitmap.bmHeight;
        }
    } else if (iconInfo.hbmMask) {
        if (GetObject(iconInfo.hbmMask, sizeof(bitmap), &bitmap) > 0) {
            desc.Width = bitmap.bmWidth;
            desc.Height = bitmap.bmHeight / 2;
        }
    }

    // Delete the bitmaps once we no longer need them.
    DeleteObject(iconInfo.hbmColor);
    DeleteObject(iconInfo.hbmMask);

    if (desc.Width == 0 || desc.Height == 0) {
        return;
    }

    // See if we need to recreate the texture (e.g. if the icon changes size...is that possible?)
    // At the very least we need it for 1st time initialization.
    if (_desc.Width != desc.Width || _desc.Height != _desc.Height) {
        _texture.attach(renderer->createTexture2D(desc));

        HRESULT hr = _texture->QueryInterface(__uuidof(IDXGISurface1), (void**)&_surface);        
        if (hr != S_OK) {
            LOG_ERROR("Failed to get IDXGISurface1 for mouse cursor texture" << hr << std::endl);
            return;
        }

        HDC hdc;
        hr = _surface->GetDC(false, &hdc);
        if (hr != S_OK) {
            LOG_WARNING("Failed to get DC for mouse cursor." << std::endl);
            return;
        }

        bool ok = false;
        try {
            ok = DrawIconEx(
                hdc,                                                        // hdc
                0,                                                          // xLeft
                0,                                                          // yTop
                info.hCursor,                                               // hIcon (cursor equivalent to icon)
                0,                                                          // cxWidth
                0,                                                          // cyWidth
                0,                                                          // istepIfAniCur
                nullptr,                                                    // hbrFlickerFreeDraw
                DI_NORMAL | DI_DEFAULTSIZE                                  // diFlags
            );
        } catch (std::exception& ex) {
            LOG_ERROR("Failed to draw cursor [exception]: " << ex.what() << std::endl);
        }
        _surface->ReleaseDC(nullptr);

        if (ok) {
            _model->setTexture(renderer->device(), context, _texture.get(), true);
            _desc = desc;
        } else {
            LOG_WARNING("Failed to draw cursor: " << shared::errors::getWin32ErrorAsString() << std::endl);
            _texture.reset();
            _surface.reset();
        }
    }

    if (_texture) {
        _model->clearXform();
        
        const auto windowWidth = windowRes.right - windowRes.left;
        const auto windowHeight = windowRes.bottom - windowRes.top;

        // (X,Y) position in the initial window. We need to translate that into normalized
        // device coordinates so that we can properly scale and translate the model.
        const auto left = cursorPosition.x - windowRes.left - iconInfo.xHotspot;
        const auto top = cursorPosition.y - windowRes.top - iconInfo.yHotspot;

        // The extra bits are for transforming the range from [0, 1] to [-1, 1]. And in the case of the Y coordinate, to [1, -1].
        const auto ndcLeft = static_cast<float>(left) / windowWidth * 2.f - 1.f;
        const auto ndcTop = static_cast<float>(top) / windowHeight * -2.f + 1.f;

        // Transform from [0, 1] to [0, 2].
        const auto ndcWidth = static_cast<float>(_desc.Width) / windowWidth;
        const auto ndcHeight = static_cast<float>(_desc.Height) / windowHeight;

        // The *center* of the rectangle for the icon is currently at (0.f, 0.f).
        // We want the top-left corner to be at (ndcLeft, ndcTop). Which means the translation is
        // (ndcLeft, ndcTop) - (ndcWidth, ndcHeight) / 2
        _model->setTranslation(DirectX::XMFLOAT3(ndcLeft - ndcWidth, ndcTop - ndcHeight, 0.f));

        _model->setScale(DirectX::XMFLOAT3(ndcWidth, ndcHeight, 1.f));
        _model->setVisibility(true);
    }
}

void MouseCursorLayer::finalizeAssetsForRenderer(service::renderer::D3d11Renderer* renderer) {
    // Create a model that we can add to the renderer. Its texture and size/position
    // will be set in updateAt prior to rendering.
    _model = service::renderer::createFullScreenQuad(renderer->device());
    _model->setVisibility(false);
    _model->setOpacity(1.f);
    renderer->addModelToScene(_model);
}

}