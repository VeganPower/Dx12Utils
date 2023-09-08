
#include "common.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include <stdexcept>
#include <memory>

using Microsoft::WRL::ComPtr;

#include "winapp.h"

constexpr void ThrowIfFailed(HRESULT result)
{
    if (result != S_OK)
        throw std::runtime_error("Meh");
}

class MyApp: public AppListener
{
public:
    MyApp(){}
    virtual ~MyApp()
    {
    }

    void setup_dx12_device(HWND hwnd, Resolution size)
    {
        UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
        // Enable the debug layer (requires the Graphics Tools "optional feature").
        // NOTE: Enabling the debug layer after device creation will invalidate the active device.
        {
            ComPtr<ID3D12Debug> debug_controller;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller))))
            {
                debug_controller->EnableDebugLayer();

                // Enable additional debug layers.
                dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
            }
        }
#endif

        ComPtr<IDXGIFactory4> factory;
        ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

        if (0)//m_useWarpDevice)
        {
            ComPtr<IDXGIAdapter> warpAdapter;
            ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

            ThrowIfFailed(D3D12CreateDevice(
                warpAdapter.Get(),
                D3D_FEATURE_LEVEL_11_0,
                IID_PPV_ARGS(&device)
                ));
        }
        else
        {
            ComPtr<IDXGIAdapter1> hardwareAdapter;
            GetHardwareAdapter(factory.Get(), &hardwareAdapter);

            ThrowIfFailed(D3D12CreateDevice(
                hardwareAdapter.Get(),
                D3D_FEATURE_LEVEL_11_0,
                IID_PPV_ARGS(&device)
                ));
        }

        // Describe and create the command queue.
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&command_queue)));

        // Describe and create the swap chain.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount      = FrameCount;
        swapChainDesc.Width            = size.width;
        swapChainDesc.Height           = size.height;
        swapChainDesc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferUsage      = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect       = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;

        ComPtr<IDXGISwapChain1> swapChain;
        ThrowIfFailed(factory->CreateSwapChainForHwnd(
            command_queue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
            hwnd, &swapChainDesc,
            nullptr, nullptr, &swapChain));

        // This sample does not support fullscreen transitions.
        ThrowIfFailed(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));

        ThrowIfFailed(swapChain.As(&swap_chain));
        frame_index = swap_chain->GetCurrentBackBufferIndex();

        // Create descriptor heaps.
        {
            // Describe and create a render target view (RTV) descriptor heap.
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
            rtvHeapDesc.NumDescriptors = FrameCount;
            rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtv_heap)));

            rtv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        }

        // Create frame resources.
        {
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtv_heap->GetCPUDescriptorHandleForHeapStart());

            // Create a RTV for each frame.
            for (UINT n = 0; n < FrameCount; n++)
            {
                ThrowIfFailed(swap_chain->GetBuffer(n, IID_PPV_ARGS(&render_targets[n])));
                device->CreateRenderTargetView(render_targets[n].Get(), nullptr, rtvHandle);
                // rtvHandle.Offset(1, rtv_descriptor_size);
                rtvHandle.ptr = SIZE_T(INT64(rtvHandle.ptr) + INT64(rtv_descriptor_size));
            }
        }

        ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator)));

        ///
        ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator.Get(), nullptr, IID_PPV_ARGS(&command_list)));

        // Command lists are created in the recording state, but there is nothing
        // to record yet. The main loop expects it to be closed, so close it now.
        ThrowIfFailed(command_list->Close());

        // Create synchronization objects.
        {
            ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
            fence_value = 1;

            // Create an event handle to use for frame synchronization.
            fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            if (fence_event == nullptr)
            {
                ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
            }
        }
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
        // Record all the commands we need to render the scene into the command list.
        PopulateCommandList();

        // Execute the command list.
        ID3D12CommandList* ppCommandLists[] = { command_list.Get() };
        command_queue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        // Present the frame.
        ThrowIfFailed(swap_chain->Present(1, 0));

        WaitForPreviousFrame();
    }

    virtual void OnDestroy() override
    {
        // Ensure that the GPU is no longer referencing resources that are about to be
        // cleaned up by the destructor.
        WaitForPreviousFrame();

        CloseHandle(fence_event);
    }

    void PopulateCommandList()
    {
        // Command list allocators can only be reset when the associated
        // command lists have finished execution on the GPU; apps should use
        // fences to determine GPU execution progress.
        ThrowIfFailed(command_allocator->Reset());

        // However, when ExecuteCommandList() is called on a particular command
        // list, that command list can then be reset at any time and must be before
        // re-recording.
        ThrowIfFailed(command_list->Reset(command_allocator.Get(), pipeline_state.Get()));

        // Indicate that the back buffer will be used as a render target.
        D3D12_RESOURCE_BARRIER bb_barrier{};
        bb_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        bb_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        bb_barrier.Transition.pResource = render_targets[frame_index].Get();
        bb_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        bb_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        bb_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        command_list->ResourceBarrier(1, &bb_barrier);

        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtv_heap->GetCPUDescriptorHandleForHeapStart();
        rtvHandle.ptr = SIZE_T(INT64(rtvHandle.ptr) + frame_index * INT64(rtv_descriptor_size));
        //, frame_index, rtv_descriptor_size);

        // Record commands.
        const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
        command_list->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

        // Indicate that the back buffer will now be used to present.
        D3D12_RESOURCE_BARRIER present_barrier{};
        present_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        present_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        present_barrier.Transition.pResource = render_targets[frame_index].Get();
        present_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        present_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        present_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        command_list->ResourceBarrier(1, &present_barrier);

        ThrowIfFailed(command_list->Close());
    }

    void WaitForPreviousFrame()
    {
        // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
        // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
        // sample illustrates how to use fences for efficient resource usage and to
        // maximize GPU utilization.

        // Signal and increment the fence value.
        const UINT64 fence_data = fence_value;
        ThrowIfFailed(command_queue->Signal(fence.Get(), fence_data));
        fence_value++;

        // Wait until the previous frame is finished.
        if (fence->GetCompletedValue() < fence_data)
        {
            ThrowIfFailed(fence->SetEventOnCompletion(fence_data, fence_event));
            WaitForSingleObject(fence_event, INFINITE);
        }

        frame_index = swap_chain->GetCurrentBackBufferIndex();
    }


    void GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter = false)
    {
        *ppAdapter = nullptr;

        ComPtr<IDXGIAdapter1> adapter;

        ComPtr<IDXGIFactory6> factory6;
        if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
        {
            for (
                UINT adapterIndex = 0;
                SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                    adapterIndex,
                    requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                    IID_PPV_ARGS(&adapter)));
                ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver adapter.
                    // If you want a software adapter, pass in "/warp" on the command line.
                    continue;
                }

                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                {
                    break;
                }
            }
        }

        if(adapter.Get() == nullptr)
        {
            for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver adapter.
                    // If you want a software adapter, pass in "/warp" on the command line.
                    continue;
                }

                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                {
                    break;
                }
            }
        }

        *ppAdapter = adapter.Detach();
    }

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
    UINT rtv_descriptor_size;

    UINT                frame_index;
    HANDLE              fence_event;
    ComPtr<ID3D12Fence> fence;
    UINT64              fence_value;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR argv, int cmd_show)
{
    auto app = std::make_unique<MyApp>();
    Window window = WindowBuilder(hInstance)
        .with_title("Test")
        .with_listener(app.get());
    app->setup_dx12_device(window.handler(), window.size());

    window.show(cmd_show);
    window.run();
    // Parse the command line parameters
    // int argc;
    // LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    // pSample->ParseCommandLineArgs(argv, argc);
    // LocalFree(argv);

    return 0;
}