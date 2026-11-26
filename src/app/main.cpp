// Veyreon Injector - C++ Edition
// Stack: DirectX 11 + Dear ImGui, Windows 10+, borderless window with per-pixel transparency.
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <dcomp.h>

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_dx11.h"

#include "ui/ui.h"

// ─── D3D11 globals ────────────────────────────────────────────────────────────
static ID3D11Device*           g_device       = nullptr;
static ID3D11DeviceContext*    g_context      = nullptr;
static IDXGISwapChain1*        g_chain        = nullptr;
static ID3D11RenderTargetView* g_rtv          = nullptr;
static IDCompositionDevice*    g_dcomp        = nullptr;
static IDCompositionTarget*    g_dcompTarget  = nullptr;
static IDCompositionVisual*    g_dcompVisual  = nullptr;

// Overall UI size. Fonts, spacing and the window all scale from this.
static const float kUiScale = 0.75f;

// ─── D3D helpers ─────────────────────────────────────────────────────────────
// A composition swap chain with premultiplied alpha gives rounded, transparent corners.
static bool CreateDevice(HWND hwnd, int width, int height) {
    D3D_FEATURE_LEVEL fl;
    const D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };
    HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                                   levels, 1, D3D11_SDK_VERSION, &g_device, &fl, &g_context);
    if (FAILED(hr))
        hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                               levels, 1, D3D11_SDK_VERSION, &g_device, &fl, &g_context);
    if (FAILED(hr)) return false;

    IDXGIDevice*   dxgiDevice = nullptr;
    IDXGIAdapter*  adapter    = nullptr;
    IDXGIFactory2* factory    = nullptr;
    hr = g_device->QueryInterface(IID_PPV_ARGS(&dxgiDevice));
    if (SUCCEEDED(hr)) hr = dxgiDevice->GetAdapter(&adapter);
    if (SUCCEEDED(hr)) hr = adapter->GetParent(IID_PPV_ARGS(&factory));

    DXGI_SWAP_CHAIN_DESC1 sd = {};
    sd.Width            = width;
    sd.Height           = height;
    sd.Format           = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.SampleDesc.Count = 1;
    sd.BufferUsage      = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount      = 2;
    sd.SwapEffect       = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    sd.AlphaMode        = DXGI_ALPHA_MODE_PREMULTIPLIED;
    if (SUCCEEDED(hr)) hr = factory->CreateSwapChainForComposition(g_device, &sd, nullptr, &g_chain);
    if (SUCCEEDED(hr)) hr = DCompositionCreateDevice(dxgiDevice, IID_PPV_ARGS(&g_dcomp));
    if (SUCCEEDED(hr)) hr = g_dcomp->CreateTargetForHwnd(hwnd, TRUE, &g_dcompTarget);
    if (SUCCEEDED(hr)) hr = g_dcomp->CreateVisual(&g_dcompVisual);
    if (SUCCEEDED(hr)) hr = g_dcompVisual->SetContent(g_chain);
    if (SUCCEEDED(hr)) hr = g_dcompTarget->SetRoot(g_dcompVisual);
    if (SUCCEEDED(hr)) hr = g_dcomp->Commit();

    if (factory)    factory->Release();
    if (adapter)    adapter->Release();
    if (dxgiDevice) dxgiDevice->Release();
    if (FAILED(hr)) return false;

    ID3D11Texture2D* back = nullptr;
    if (FAILED(g_chain->GetBuffer(0, IID_PPV_ARGS(&back)))) return false;
    hr = g_device->CreateRenderTargetView(back, nullptr, &g_rtv);
    back->Release();
    return SUCCEEDED(hr);
}

static void CleanupDevice() {
    if (g_rtv)         { g_rtv->Release();         g_rtv         = nullptr; }
    if (g_chain)       { g_chain->Release();       g_chain       = nullptr; }
    if (g_dcompVisual) { g_dcompVisual->Release(); g_dcompVisual = nullptr; }
    if (g_dcompTarget) { g_dcompTarget->Release(); g_dcompTarget = nullptr; }
    if (g_dcomp)       { g_dcomp->Release();       g_dcomp       = nullptr; }
    if (g_context)     { g_context->Release();     g_context     = nullptr; }
    if (g_device)      { g_device->Release();      g_device      = nullptr; }
}

// ─── Window procedure ─────────────────────────────────────────────────────────
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wp, lp))
        return true;

    switch (msg) {
    case WM_SYSCOMMAND:
        if ((wp & 0xFFF0) == SC_KEYMENU) return 0; // suppress Alt menu flash
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

// ─── Entry point ─────────────────────────────────────────────────────────────
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int) {
    ImGui_ImplWin32_EnableDpiAwareness();
    const int sx = GetSystemMetrics(SM_CXSCREEN);
    const int sy = GetSystemMetrics(SM_CYSCREEN);

    // Shrink only if the display is too small for the preferred scale.
    float scale = kUiScale;
    const float fit = (float)sy * 0.92f / 768.0f;
    if (fit < scale) scale = fit;
    const int winW = (int)(1108 * scale), winH = (int)(768 * scale);

    WNDCLASSEXW wc = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_CLASSDC;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"VeyreonInjectorCPP";
    RegisterClassExW(&wc);

    // NOREDIRECTIONBITMAP: only what DirectComposition presents is visible.
    HWND hwnd = CreateWindowExW(
        WS_EX_NOREDIRECTIONBITMAP, wc.lpszClassName, L"Veyreon Injector",
        WS_POPUP | WS_MINIMIZEBOX,
        (sx - winW) / 2, (sy - winH) / 2, winW, winH,
        nullptr, nullptr, hInst, nullptr);

    if (!hwnd || !CreateDevice(hwnd, winW, winH)) {
        MessageBoxW(nullptr,
            L"Failed to initialise DirectX 11.\n\n"
            L"Your system may be missing the DirectX runtime.\n"
            L"Download it from:\n"
            L"microsoft.com/en-us/download/details.aspx?id=35",
            L"Veyreon Injector – Missing Requirement",
            MB_ICONERROR | MB_OK);
        CleanupDevice();
        UnregisterClassW(wc.lpszClassName, hInst);
        return 1;
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_device, g_context);

    if (!UI::Init(scale)) {
        MessageBoxW(nullptr, L"Failed to initialise the interface.", L"Veyreon Injector", MB_ICONERROR);
        return 1;
    }

    // Main loop. Frames are drawn only for a short while after input, long enough
    // for animations to settle; otherwise the thread sleeps and uses no CPU or GPU.
    const ULONGLONG kAwakeMs = 800;
    ULONGLONG awakeUntil = GetTickCount64() + 1500;   // covers the launch fade
    POINT dragOffset = {};
    bool dragging = false;
    MSG msg = {};
    while (true) {
        const bool hidden = IsIconic(hwnd) || !IsWindowVisible(hwnd);
        if (hidden || GetTickCount64() >= awakeUntil) {
            // A focused text field still wakes twice a second to blink the caret.
            const DWORD timeout = !hidden && ImGui::GetIO().WantTextInput ? 500 : INFINITE;
            MsgWaitForMultipleObjectsEx(0, nullptr, timeout, QS_ALLINPUT, MWMO_INPUTAVAILABLE);
        }

        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            if (msg.message == WM_QUIT) goto cleanup;
            awakeUntil = GetTickCount64() + kAwakeMs;
        }

        // Nothing to draw while minimized or hidden.
        if (IsIconic(hwnd) || !IsWindowVisible(hwnd))
            continue;
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        // The first frame after sleeping would otherwise skip every animation.
        if (ImGui::GetIO().DeltaTime > 1.0f / 30.0f) ImGui::GetIO().DeltaTime = 1.0f / 30.0f;
        ImGui::NewFrame();

        UI::Render(hwnd);

        // Titlebar drag. The window is moved here, not by the system caption.
        if (UI::TitleDragging()) {
            POINT cur;
            RECT rc;
            GetCursorPos(&cur);
            GetWindowRect(hwnd, &rc);
            if (!dragging) {
                dragOffset = { cur.x - rc.left, cur.y - rc.top };
                dragging = true;
            }
            const int nx = cur.x - dragOffset.x, ny = cur.y - dragOffset.y;
            if (nx != rc.left || ny != rc.top)
                SetWindowPos(hwnd, nullptr, nx, ny, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        } else {
            dragging = false;
        }

        // Keep drawing while a button is held, even if the mouse is still.
        if (ImGui::IsAnyMouseDown()) awakeUntil = GetTickCount64() + kAwakeMs;

        ImGui::Render();

        // Launch fade over the desktop.
        const float opacity = UI::IntroOpacity();
        if (opacity < 1.0f) {
            ImDrawData* dd = ImGui::GetDrawData();
            for (int l = 0; l < dd->CmdListsCount; l++) {
                for (ImDrawVert& v : dd->CmdLists[l]->VtxBuffer) {
                    const ImU32 a = (ImU32)(((v.col >> IM_COL32_A_SHIFT) & 0xFF) * opacity);
                    v.col = (v.col & ~IM_COL32_A_MASK) | (a << IM_COL32_A_SHIFT);
                }
            }
        }

        const D3D11_VIEWPORT vp = { 0, 0, (float)winW, (float)winH, 0, 1 };
        const float clear[4] = { 0, 0, 0, 0 };
        g_context->RSSetViewports(1, &vp);
        g_context->OMSetRenderTargets(1, &g_rtv, nullptr);
        g_context->ClearRenderTargetView(g_rtv, clear);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_chain->Present(1, 0); // vsync on
    }

cleanup:
    UI::Shutdown();
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDevice();
    DestroyWindow(hwnd);
    UnregisterClassW(wc.lpszClassName, hInst);
    return 0;
}
