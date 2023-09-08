
#include "common.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include  <memory>

using Microsoft::WRL::ComPtr;

#include "winapp.h"


class Dx12Base
{
    static const UINT FrameCount = 2;

    // Pipeline objects.
    ComPtr<IDXGISwapChain3>           swap_chain;
    ComPtr<ID3D12Device>              device;
    ComPtr<ID3D12Resource>            render_targets[FrameCount];
    ComPtr<ID3D12CommandAllocator>    command_allocator;
    ComPtr<ID3D12CommandQueue>        command_queue;
    ComPtr<ID3D12DescriptorHeap>      rtv_heap;
    ComPtr<ID3D12PipelineState>       pipeline_state;
    ComPtr<ID3D12GraphicsCommandList> command_list;

    UINT                frame_index;
    HANDLE              fence_event;
    ComPtr<ID3D12Fence> fence;
    UINT64              fence_value;
};

class MyApp: public AppListener
{
public:
    MyApp(){}
    virtual ~MyApp()
    {
    }
private:
    virtual void OnKeyDown(char ch) override
    {

    }
    virtual void OnKeyUp(char ch) override
    {

    }
    virtual void OnUpdate() override
    {

    }
    virtual void OnRender() override
    {

    }

    Dx12Base dx12;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR argv, int cmd_show)
{
    auto app = std::make_unique<MyApp>();
    WinApp window = WindowBuilder(hInstance)
        .with_title("Test")
        .with_listener(app.get());

    window.show(cmd_show);
    window.run();
    // Parse the command line parameters
    // int argc;
    // LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    // pSample->ParseCommandLineArgs(argv, argc);
    // LocalFree(argv);

    return 0;
}