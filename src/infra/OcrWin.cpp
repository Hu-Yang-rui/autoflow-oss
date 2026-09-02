#include "OcrWin.h"

#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>
#include <wrl/event.h>

#include <windows.foundation.h>
#include <windows.foundation.collections.h>
#include <windows.media.ocr.h>
#include <windows.graphics.imaging.h>
#include <windows.storage.streams.h>
#include <windows.globalization.h>
#include <roapi.h>
#include <robuffer.h>

#include <QCoreApplication>
#include <QString>

#include <cstring>

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Media::Ocr;
using namespace ABI::Windows::Graphics::Imaging;
using namespace ABI::Windows::Storage::Streams;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::Globalization;

namespace autoflow {

// RAII：进入 WinRT 运行环境，离开时成对释放（仅在本次成功初始化时 Uninitialize）
struct RoInitGuard {
    bool uninit;
    RoInitGuard() : uninit(SUCCEEDED(RoInitialize(RO_INIT_MULTITHREADED))) {}
    ~RoInitGuard() { if (uninit) RoUninitialize(); }
};

// 同步等待 IAsyncOperation<TAsync*> 完成并取出结果（TResult 为 ABI 接口）
template <typename TAsync, typename TResult>
static HRESULT waitForOperation(ComPtr<IAsyncOperation<TAsync*>>& op, ComPtr<TResult>& result) {
    HANDLE done = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!done) return E_FAIL;
    HRESULT hr = op->put_Completed(
        Callback<IAsyncOperationCompletedHandler<TAsync*>>(
            [done](IAsyncOperation<TAsync*>* /*op*/, AsyncStatus /*status*/) -> HRESULT {
                SetEvent(done);
                return S_OK;
            }).Get());
    if (SUCCEEDED(hr)) {
        WaitForSingleObject(done, INFINITE);
        hr = op->GetResults(result.GetAddressOf());
    }
    CloseHandle(done);
    return hr;
}

std::vector<OcrWordInfo> ocrWords(const QImage& src, std::string& err) {
    std::vector<OcrWordInfo> out;
    if (src.isNull()) {
        err = QCoreApplication::translate("Infra", "空图片").toStdString();
        return out;
    }

    // Windows OCR 需 Bgra8；QImage::Format_RGB32 内存布局恰为 B,G,R,A
    QImage img = (src.format() == QImage::Format_RGB32)
        ? src : src.convertToFormat(QImage::Format_RGB32);

    RoInitGuard ro;

    const int w = img.width();
    const int h = img.height();
    const int stride = w * 4;

    // 1) 像素 → 连续 BGRA buffer（Windows.Storage.Streams.Buffer）
    ComPtr<IBuffer> buffer;
    HRESULT hr;
    {
        ComPtr<IBufferFactory> bufferFactory;
        hr = RoGetActivationFactory(
            HStringReference(L"Windows.Storage.Streams.Buffer").Get(),
            __uuidof(IBufferFactory), &bufferFactory);
        if (FAILED(hr)) { err = QCoreApplication::translate("Infra", "创建图像缓冲组件失败").toStdString(); return out; }
        hr = bufferFactory->Create((UINT32)(stride * h), &buffer);
        if (FAILED(hr)) { err = QCoreApplication::translate("Infra", "创建图像缓冲失败").toStdString(); return out; }
    }
    ComPtr<Windows::Storage::Streams::IBufferByteAccess> byteAccess;
    BYTE* pData = nullptr;
    if (FAILED(buffer.As(&byteAccess))) { err = QCoreApplication::translate("Infra", "图像缓冲访问失败").toStdString(); return out; }
    if (FAILED(byteAccess->Buffer(&pData))) { err = QCoreApplication::translate("Infra", "图像缓冲指针失败").toStdString(); return out; }
    for (int y = 0; y < h; ++y)
        std::memcpy(pData + (size_t)y * stride, img.constScanLine(y), (size_t)stride);
    // Buffer 创建后 Length 默认为 0，必须显式设置为实际字节数，CreateCopyFromBuffer 才会读取像素
    buffer->put_Length((UINT32)(stride * h));

    // 2) SoftwareBitmap（Bgra8）
    ComPtr<ISoftwareBitmap> bitmap;
    {
        ComPtr<ISoftwareBitmapStatics> statics;
        hr = RoGetActivationFactory(
            HStringReference(L"Windows.Graphics.Imaging.SoftwareBitmap").Get(),
            __uuidof(ISoftwareBitmapStatics), &statics);
        if (FAILED(hr)) { err = QCoreApplication::translate("Infra", "OCR 位图组件不可用").toStdString(); return out; }
        hr = statics->CreateCopyFromBuffer(buffer.Get(), BitmapPixelFormat_Bgra8, w, h, &bitmap);
        if (FAILED(hr)) { err = QCoreApplication::translate("Infra", "创建 OCR 位图失败").toStdString(); return out; }
    }

    // 3) OcrEngine：优先中文，其次用户语言
    ComPtr<IOcrEngine> engine;
    {
        ComPtr<IOcrEngineStatics> statics;
        hr = RoGetActivationFactory(
            HStringReference(L"Windows.Media.Ocr.OcrEngine").Get(),
            __uuidof(IOcrEngineStatics), &statics);
        if (FAILED(hr)) { err = QCoreApplication::translate("Infra", "OCR 引擎组件不可用").toStdString(); return out; }

        ComPtr<ILanguage> zh;
        {
            ComPtr<ILanguageFactory> lf;
            if (SUCCEEDED(RoGetActivationFactory(
                    HStringReference(L"Windows.Globalization.Language").Get(),
                    __uuidof(ILanguageFactory), &lf))) {
                lf->CreateLanguage(HStringReference(L"zh-Hans").Get(), &zh);
            }
        }
        if (zh) {
            ComPtr<IOcrEngine> e;
            if (SUCCEEDED(statics->TryCreateFromLanguage(zh.Get(), &e)) && e) engine = e;
        }
        if (!engine) {
            ComPtr<IOcrEngine> e;
            if (SUCCEEDED(statics->TryCreateFromUserProfileLanguages(&e)) && e) engine = e;
        }
        if (!engine) {
            err = QCoreApplication::translate("Infra", "OCR 引擎不可用（请在系统设置中安装中文 OCR 语言包）").toStdString();
            return out;
        }
    }

    // 4) 识别（异步，返回接口 IOcrResult）
    ComPtr<IOcrResult> ocrResult;
    {
        ComPtr<IAsyncOperation<OcrResult*>> op;
        hr = engine->RecognizeAsync(bitmap.Get(), &op);
        if (FAILED(hr)) { err = QCoreApplication::translate("Infra", "OCR 识别失败").toStdString(); return out; }
        hr = waitForOperation<OcrResult, IOcrResult>(op, ocrResult);
        if (FAILED(hr)) { err = QCoreApplication::translate("Infra", "OCR 等待结果失败").toStdString(); return out; }
    }

    // 5) 遍历行/词，收集文字 + 坐标
    ComPtr<IVectorView<OcrLine*>> lines;
    if (FAILED(ocrResult->get_Lines(&lines))) { err = QCoreApplication::translate("Infra", "读取 OCR 行失败").toStdString(); return out; }
    UINT32 lineCount = 0;
    lines->get_Size(&lineCount);
    for (UINT32 i = 0; i < lineCount; ++i) {
        ComPtr<IOcrLine> line;
        if (FAILED(lines->GetAt(i, &line))) continue;
        ComPtr<IVectorView<OcrWord*>> words;
        if (FAILED(line->get_Words(&words))) continue;
        UINT32 wordCount = 0;
        words->get_Size(&wordCount);
        for (UINT32 j = 0; j < wordCount; ++j) {
            ComPtr<IOcrWord> word;
            if (FAILED(words->GetAt(j, &word))) continue;
            HString text;
            Rect rect{};
            if (FAILED(word->get_Text(text.GetAddressOf()))) continue;
            if (FAILED(word->get_BoundingRect(&rect))) continue;

            OcrWordInfo o;
            const wchar_t* wt = text.GetRawBuffer(nullptr);
            o.text = wt ? QString::fromWCharArray(wt).toStdString() : std::string();
            o.x = (int)rect.X;
            o.y = (int)rect.Y;
            o.w = (int)rect.Width;
            o.h = (int)rect.Height;
            out.push_back(std::move(o));
        }
    }

    return out;
}

} // namespace autoflow
