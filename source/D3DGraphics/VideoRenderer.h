#pragma once

#include "D3DGraphics/Common/DeviceResources.h"
#include "D3DGraphics/Common/StepTimer.h"
#include "D3DGraphics/ATG Tool Kit/d3dx12.h"

class VideoRenderer final : public DX::IDeviceNotify
{
public:

    VideoRenderer() noexcept(false);
    ~VideoRenderer();

    VideoRenderer(VideoRenderer&&) = default;
    VideoRenderer& operator= (VideoRenderer&&) = default;

    VideoRenderer(VideoRenderer const&) = delete;
    VideoRenderer& operator= (VideoRenderer const&) = delete;

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    D3D12_RESOURCE_DESC ChooseTexture();

    // Basic render loop
    void Tick();

    // IDeviceNotify
    virtual void OnDeviceLost() override;
    virtual void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize(int& width, int& height) const;

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    // Vars to choose whether to display GameLink or not
    const uint32_t m_framesDelay = 60;
    uint32_t m_previousFrameCount;    // Last render frame count
    UINT16 m_previousGameLinkFrameSequence;
    bool m_useGameLink;

    // Background image when GameLink isn't available
    std::vector<uint8_t> m_bgImage;
    uint32_t m_bgImageWidth;
    uint32_t m_bgImageHeight;

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;

    // Input devices.
    std::unique_ptr<DirectX::GamePad>       m_gamePad;
    std::unique_ptr<DirectX::Keyboard>      m_keyboard;

    // DirectXTK objects.
    std::unique_ptr<DirectX::GraphicsMemory>        m_graphicsMemory;

    // Direct3D 12 objects
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>    m_srvHeap;
    Microsoft::WRL::ComPtr<ID3D12RootSignature>     m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState>     m_pipelineState;
    Microsoft::WRL::ComPtr<ID3D12Resource>          m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource>          m_indexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource>          m_texture;
    D3D12_VERTEX_BUFFER_VIEW                        m_vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW                         m_indexBufferView;
};

