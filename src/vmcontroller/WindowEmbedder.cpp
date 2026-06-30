/**
 * WindowEmbedder.cpp
 * 
 * Implementation of Win32 API window embedding for Scrcpy
 */

#include "vmcontroller/WindowEmbedder.hpp"
#include <QDebug>
#include <QApplication>
#include <QWindow>

#ifdef _WIN32

// Required for SetParent
#include <qt_windows.h>
#include <windowsx.hpp>

namespace VirtualPhonePro {

// ============================================================================
// Singleton Implementation
// ============================================================================
WindowEmbedder& WindowEmbedder::getInstance() {
    static WindowEmbedder instance;
    return instance;
}

// ============================================================================
// Constructor & Destructor
// ============================================================================
WindowEmbedder::WindowEmbedder(QObject* parent)
    : QObject(parent)
{
}

WindowEmbedder::~WindowEmbedder() {
    shutdown();
}

// ============================================================================
// Initialization
// ============================================================================
bool WindowEmbedder::initialize() {
    qInfo() << "[WindowEmbedder] Initialized successfully";
    return true;
}

void WindowEmbedder::shutdown() {
    qInfo() << "[WindowEmbedder] Shutting down...";
    
    // Stop all monitoring
    stopAllMonitoring();
    
    // Unembed all windows
    unembedAllWindows();
    
    qInfo() << "[WindowEmbedder] Shutdown complete";
}

// ============================================================================
// Window Search
// ============================================================================
HWND WindowEmbedder::findWindowByTitle(const QString& title) {
    // Convert QString to wide string
    std::wstring wTitle = title.toStdWString();
    
    // FindWindowW searches top-level windows only
    HWND hwnd = FindWindowW(nullptr, wTitle.c_str());
    
    if (hwnd) {
        emit windowFound(title, hwnd);
        qDebug() << "[WindowEmbedder] Found window:" << title << "HWND:" << QString::number((quintptr)hwnd, 16);
    } else {
        qDebug() << "[WindowEmbedder] Window not found:" << title;
    }
    
    return hwnd;
}

HWND WindowEmbedder::findWindowByClass(const QString& className) {
    std::wstring wClass = className.toStdWString();
    HWND hwnd = FindWindowW(reinterpret_cast<LPCWSTR>(wClass.c_str()), nullptr);
    return hwnd;
}

QList<HWND> WindowEmbedder::findAllWindows(const QString& partialTitle) {
    QList<HWND> results;
    std::wstring wPartial = partialTitle.toStdWString();
    
    struct FindData {
        QString partial;
        QList<HWND>* results;
    } findData = { partialTitle, &results };
    
    // Use EnumWindows to find all matching windows
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        FindData* data = reinterpret_cast<FindData*>(lParam);
        
        // Get window title
        wchar_t title[512];
        int len = GetWindowTextW(hwnd, title, 511);
        if (len > 0) {
            QString windowTitle = QString::fromWCharArray(title);
            if (windowTitle.contains(data->partial, Qt::CaseInsensitive)) {
                data->results->append(hwnd);
            }
        }
        
        return TRUE;
    }, reinterpret_cast<LPARAM>(&findData));
    
    return results;
}

// ============================================================================
// Window Embedding
// ============================================================================
bool WindowEmbedder::embedWindow(const QString& windowTitle, WId containerId) {
    QMutexLocker locker(&m_mutex);
    
    // Find the window
    HWND scrcpyHwnd = findWindowByTitle(windowTitle);
    if (!scrcpyHwnd) {
        // Window not found yet, might need to wait
        qWarning() << "[WindowEmbedder] Cannot embed - window not found:" << windowTitle;
        emit embeddingError(windowTitle, "Window not found");
        return false;
    }
    
    return embedWindowByHandle(scrcpyHwnd, containerId);
}

bool WindowEmbedder::embedWindowByHandle(HWND windowHandle, WId containerId) {
    QMutexLocker locker(&m_mutex);
    
    if (!windowHandle || !containerId) {
        qWarning() << "[WindowEmbedder] Invalid handle or container";
        return false;
    }
    
    // Get the Qt window from container ID
    QWindow* containerWindow = QWindow::fromWinId(containerId);
    if (!containerWindow) {
        qWarning() << "[WindowEmbedder] Cannot get container window";
        return false;
    }
    
    // Store original styles for later restoration
    LONG originalStyle = GetWindowLongPtrW(windowHandle, GWL_STYLE);
    LONG originalExStyle = GetWindowLongPtrW(windowHandle, GWL_EXSTYLE);
    m_originalStyles[windowHandle] = originalStyle;
    m_originalExStyles[windowHandle] = originalExStyle;
    
    // Get container window handle
    HWND containerHwnd = reinterpret_cast<HWND>(containerId);
    
    // Get container position and size
    RECT rect;
    GetWindowRect(containerHwnd, &rect);
    
    // Save window title
    wchar_t title[512];
    GetWindowTextW(windowHandle, title, 511);
    QString windowTitleStr = QString::fromWCharArray(title);
    
    // Remove window from its parent (usually desktop)
    SetParent(windowHandle, nullptr);
    
    // Small delay to let the window detach
    Sleep(50);
    
    // Embed into our container
    SetParent(windowHandle, containerHwnd);
    
    // Apply child window styles
    applyChildStyle(windowHandle);
    
    // Position and size the window to fill the container
    SetWindowPos(windowHandle, HWND_TOP, 
                0, 0, 
                rect.right - rect.left, 
                rect.bottom - rect.top,
                SWP_SHOWWINDOW | SWP_NOZORDER);
    
    // Update internal state
    EmbeddedWindowInfo info;
    info.hwnd = windowHandle;
    info.containerId = containerId;
    info.title = windowTitleStr;
    info.isEmbedded = true;
    info.resizeTimer = new QTimer(this);
    connect(info.resizeTimer, &QTimer::timeout, [this, windowTitleStr, containerId]() {
        updateEmbeddedWindowPosition(windowTitleStr);
    });
    
    m_embeddedWindows[windowTitleStr] = info;
    m_handleToTitle[windowHandle] = windowTitleStr;
    
    emit windowEmbedded(windowTitleStr, windowHandle);
    qInfo() << "[WindowEmbedder] Window embedded successfully:" << windowTitleStr;
    
    return true;
}

bool WindowEmbedder::unembedWindow(const QString& windowTitle) {
    QMutexLocker locker(&m_mutex);
    
    if (!m_embeddedWindows.contains(windowTitle)) {
        return false;
    }
    
    HWND hwnd = m_embeddedWindows[windowTitle].hwnd;
    
    // Restore original styles
    if (m_originalStyles.contains(hwnd)) {
        SetWindowLongPtrW(hwnd, GWL_STYLE, m_originalStyles[hwnd]);
        SetWindowLongPtrW(hwnd, GWL_EXSTYLE, m_originalExStyles[hwnd]);
        m_originalStyles.remove(hwnd);
        m_originalExStyles.remove(hwnd);
    }
    
    // Unparent from container
    SetParent(hwnd, nullptr);
    
    // Stop monitoring
    if (m_embeddedWindows[windowTitle].resizeTimer) {
        m_embeddedWindows[windowTitle].resizeTimer->stop();
        delete m_embeddedWindows[windowTitle].resizeTimer;
    }
    
    // Clean up
    m_handleToTitle.remove(hwnd);
    m_embeddedWindows.remove(windowTitle);
    
    emit windowUnembedded(windowTitle);
    qInfo() << "[WindowEmbedder] Window unembedded:" << windowTitle;
    
    return true;
}

bool WindowEmbedder::unembedAllWindows() {
    QStringList titles = m_embeddedWindows.keys();
    for (const QString& title : titles) {
        unembedWindow(title);
    }
    return true;
}

// ============================================================================
// Window Control
// ============================================================================
bool WindowEmbedder::resizeWindow(HWND hwnd, int width, int height) {
    if (!hwnd) return false;
    
    BOOL result = SetWindowPos(hwnd, HWND_TOP, 
                              0, 0, width, height,
                              SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW);
    
    if (result) {
        QString title = getWindowTitle(hwnd);
        emit windowResized(title, width, height);
    }
    
    return result != 0;
}

bool WindowEmbedder::moveWindow(HWND hwnd, int x, int y) {
    if (!hwnd) return false;
    
    return SetWindowPos(hwnd, HWND_TOP, x, y, 0, 0, 
                      SWP_NOSIZE | SWP_NOZORDER) != 0;
}

bool WindowEmbedder::setWindowSizeToContainer(HWND hwnd, WId containerId) {
    if (!hwnd || !containerId) return false;
    
    HWND containerHwnd = reinterpret_cast<HWND>(containerId);
    
    RECT rect;
    GetWindowRect(containerHwnd, &rect);
    
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    
    return resizeWindow(hwnd, width, height);
}

bool WindowEmbedder::bringToFront(HWND hwnd) {
    if (!hwnd) return false;
    
    // Bring to front in z-order
    SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    
    // Also activate the window
    SetForegroundWindow(hwnd);
    SetActiveWindow(hwnd);
    
    return true;
}

bool WindowEmbedder::hideOriginalChrome(HWND hwnd) {
    if (!hwnd) return false;
    
    // Remove title bar, border, etc.
    removeAllChrome(hwnd);
    
    return true;
}

bool WindowEmbedder::showOriginalChrome(HWND hwnd) {
    if (!hwnd) return false;
    
    // Restore original styles
    if (m_originalStyles.contains(hwnd)) {
        SetWindowLongPtrW(hwnd, GWL_STYLE, m_originalStyles[hwnd]);
        SetWindowLongPtrW(hwnd, GWL_EXSTYLE, m_originalExStyles[hwnd]);
        SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, 
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
    
    return true;
}

// ============================================================================
// Styling
// ============================================================================
bool WindowEmbedder::applyChildStyle(HWND hwnd) {
    if (!hwnd) return false;
    
    // Get current styles
    LONG style = GetWindowLongPtrW(hwnd, GWL_STYLE);
    LONG exStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    
    // Remove window-specific styles that would prevent embedding
    style &= ~(WS_POPUP);           // Remove popup (we want child)
    style |= WS_CHILD;               // Add child style
    style |= WS_VISIBLE;             // Make visible
    
    // Remove decorations
    style &= ~(WS_CAPTION);          // No caption
    style &= ~(WS_BORDER);           // No border
    style &= ~(WS_DLGFRAME);         // No dialog frame
    style &= ~(WS_SYSMENU);          // No system menu
    style &= ~(WS_MINIMIZEBOX);      // No minimize
    style &= ~(WS_MAXIMIZEBOX);      // No maximize
    style &= ~(WS_SIZEBOX);          // No resizable border
    
    // Extended styles
    exStyle &= ~(WS_EX_WINDOWEDGE);
    exStyle &= ~(WS_EX_CLIENTEDGE);
    exStyle &= ~(WS_EX_DLGMODALFRAME);
    exStyle &= ~(WS_EX_STATICEDGE);
    exStyle |= WS_EX_TRANSPARENT;    // Transparent for layering
    
    // Apply new styles
    SetWindowLongPtrW(hwnd, GWL_STYLE, style);
    SetWindowLongPtrW(hwnd, GWL_EXSTYLE, exStyle);
    
    // Notify window of style change
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, 
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    
    return true;
}

bool WindowEmbedder::removeTitleBar(HWND hwnd) {
    if (!hwnd) return false;
    
    LONG style = GetWindowLongPtrW(hwnd, GWL_STYLE);
    style &= ~(WS_CAPTION);
    SetWindowLongPtrW(hwnd, GWL_STYLE, style);
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, 
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    
    return true;
}

bool WindowEmbedder::removeBorder(HWND hwnd) {
    if (!hwnd) return false;
    
    LONG style = GetWindowLongPtrW(hwnd, GWL_STYLE);
    LONG exStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    
    style &= ~(WS_BORDER);
    style &= ~(WS_DLGFRAME);
    exStyle &= ~(WS_EX_WINDOWEDGE);
    exStyle &= ~(WS_EX_CLIENTEDGE);
    
    SetWindowLongPtrW(hwnd, GWL_STYLE, style);
    SetWindowLongPtrW(hwnd, GWL_EXSTYLE, exStyle);
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, 
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    
    return true;
}

bool WindowEmbedder::removeAllChrome(HWND hwnd) {
    if (!hwnd) return false;
    
    LONG style = GetWindowLongPtrW(hwnd, GWL_STYLE);
    LONG exStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    
    // Remove all possible chrome
    style &= ~(WS_CAPTION);
    style &= ~(WS_BORDER);
    style &= ~(WS_DLGFRAME);
    style &= ~(WS_SYSMENU);
    style &= ~(WS_MINIMIZEBOX);
    style &= ~(WS_MAXIMIZEBOX);
    style &= ~(WS_SIZEBOX);
    style &= ~(WS_POPUP);  // Ensure it's a child window
    style |= WS_CHILD;
    style |= WS_VISIBLE;
    
    exStyle &= ~(WS_EX_WINDOWEDGE);
    exStyle &= ~(WS_EX_CLIENTEDGE);
    exStyle &= ~(WS_EX_DLGMODALFRAME);
    exStyle &= ~(WS_EX_STATICEDGE);
    
    // Apply
    SetWindowLongPtrW(hwnd, GWL_STYLE, style);
    SetWindowLongPtrW(hwnd, GWL_EXSTYLE, exStyle);
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, 
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    
    return true;
}

bool WindowEmbedder::setWindowTransparent(HWND hwnd, bool transparent) {
    if (!hwnd) return false;
    
    LONG exStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    
    if (transparent) {
        exStyle |= WS_EX_TRANSPARENT;
    } else {
        exStyle &= ~(WS_EX_TRANSPARENT);
    }
    
    SetWindowLongPtrW(hwnd, GWL_EXSTYLE, exStyle);
    return true;
}

// ============================================================================
// Event Handling
// ============================================================================
bool WindowEmbedder::redirectInput(HWND sourceHwnd, HWND targetHwnd) {
    // This would require a low-level keyboard/mouse hook
    // For now, Scrcpy handles input forwarding directly
    Q_UNUSED(sourceHwnd);
    Q_UNUSED(targetHwnd);
    return true;
}

bool WindowEmbedder::forwardMouseEvents(HWND source, HWND target) {
    Q_UNUSED(source);
    Q_UNUSED(target);
    // Mouse events are handled by Qt's event system
    return true;
}

bool WindowEmbedder::forwardKeyboardEvents(HWND source, HWND target) {
    Q_UNUSED(source);
    Q_UNUSED(target);
    // Keyboard events are handled by Qt's event system
    return true;
}

// ============================================================================
// Monitoring
// ============================================================================
bool WindowEmbedder::startWindowMonitoring(const QString& windowTitle, int intervalMs) {
    if (m_monitoringTimers.contains(windowTitle)) {
        return true; // Already monitoring
    }
    
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [this, windowTitle]() {
        HWND hwnd = findWindowByTitle(windowTitle);
        
        if (hwnd && !m_embeddedWindows.contains(windowTitle)) {
            // Window appeared but not embedded - could trigger auto-embed
            qDebug() << "[WindowEmbedder] Window detected:" << windowTitle;
        } else if (!hwnd && m_embeddedWindows.contains(windowTitle)) {
            // Window disappeared while embedded
            qWarning() << "[WindowEmbedder] Embedded window disappeared:" << windowTitle;
            emit windowLost(windowTitle, m_embeddedWindows[windowTitle].hwnd);
        }
    });
    
    timer->start(intervalMs);
    m_monitoringTimers[windowTitle] = timer;
    
    return true;
}

bool WindowEmbedder::stopWindowMonitoring(const QString& windowTitle) {
    if (m_monitoringTimers.contains(windowTitle)) {
        m_monitoringTimers[windowTitle]->stop();
        delete m_monitoringTimers[windowTitle];
        m_monitoringTimers.remove(windowTitle);
        return true;
    }
    return false;
}

void WindowEmbedder::stopAllMonitoring() {
    for (auto timer : m_monitoringTimers) {
        timer->stop();
        delete timer;
    }
    m_monitoringTimers.clear();
}

// ============================================================================
// State Query
// ============================================================================
bool WindowEmbedder::isWindowEmbedded(HWND hwnd) const {
    return m_handleToTitle.contains(hwnd);
}

bool WindowEmbedder::isWindowEmbeddedByTitle(const QString& title) const {
    return m_embeddedWindows.contains(title);
}

HWND WindowEmbedder::getEmbeddedWindowHandle(const QString& title) const {
    if (m_embeddedWindows.contains(title)) {
        return m_embeddedWindows[title].hwnd;
    }
    return nullptr;
}

QString WindowEmbedder::getWindowTitle(HWND hwnd) const {
    if (!hwnd) return QString();
    
    wchar_t title[512];
    int len = GetWindowTextW(hwnd, title, 511);
    if (len > 0) {
        return QString::fromWCharArray(title);
    }
    return QString();
}

QStringList WindowEmbedder::getEmbeddedWindowTitles() const {
    return m_embeddedWindows.keys();
}

// ============================================================================
// Container Management
// ============================================================================
bool WindowEmbedder::attachContainer(const QString& windowTitle, WId containerId) {
    if (!m_embeddedWindows.contains(windowTitle)) {
        return false;
    }
    
    m_embeddedWindows[windowTitle].containerId = containerId;
    return true;
}

bool WindowEmbedder::detachContainer(const QString& windowTitle) {
    if (!m_embeddedWindows.contains(windowTitle)) {
        return false;
    }
    
    m_embeddedWindows[windowTitle].containerId = 0;
    return true;
}

bool WindowEmbedder::updateContainerSize(const QString& windowTitle) {
    return updateEmbeddedWindowPosition(windowTitle);
}

// ============================================================================
// Private Helpers
// ============================================================================
bool WindowEmbedder::setWindowStyle(HWND hwnd, DWORD addStyle, DWORD removeStyle) {
    if (!hwnd) return false;
    
    LONG style = GetWindowLongPtrW(hwnd, GWL_STYLE);
    style |= addStyle;
    style &= ~removeStyle;
    SetWindowLongPtrW(hwnd, GWL_STYLE, style);
    
    return true;
}

bool WindowEmbedder::updateEmbeddedWindowPosition(const QString& title) {
    if (!m_embeddedWindows.contains(title)) {
        return false;
    }
    
    const EmbeddedWindowInfo& info = m_embeddedWindows[title];
    
    if (!info.containerId) {
        return false;
    }
    
    HWND containerHwnd = reinterpret_cast<HWND>(info.containerId);
    
    RECT rect;
    GetWindowRect(containerHwnd, &rect);
    
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    
    // Update embedded window size
    SetWindowPos(info.hwnd, HWND_TOP, 
                0, 0, width, height,
                SWP_NOZORDER | SWP_SHOWWINDOW);
    
    emit windowResized(title, width, height);
    
    return true;
}

void WindowEmbedder::cleanupEmbeddedWindow(const QString& title) {
    if (m_embeddedWindows.contains(title)) {
        const EmbeddedWindowInfo& info = m_embeddedWindows[title];
        
        // Restore styles
        if (m_originalStyles.contains(info.hwnd)) {
            SetWindowLongPtrW(info.hwnd, GWL_STYLE, m_originalStyles[info.hwnd]);
            SetWindowLongPtrW(info.hwnd, GWL_EXSTYLE, m_originalExStyles[info.hwnd]);
            m_originalStyles.remove(info.hwnd);
            m_originalExStyles.remove(info.hwnd);
        }
        
        // Unparent
        SetParent(info.hwnd, nullptr);
        
        // Cleanup timer
        if (info.resizeTimer) {
            info.resizeTimer->stop();
            delete info.resizeTimer;
        }
        
        // Remove from maps
        m_handleToTitle.remove(info.hwnd);
        m_embeddedWindows.remove(title);
    }
}

} // namespace VirtualPhonePro

#else
// Non-Windows stub implementation
namespace VirtualPhonePro {
    bool WindowEmbedder::initialize() { return false; }
    void WindowEmbedder::shutdown() {}
    bool WindowEmbedder::embedWindow(const QString&, WId) { return false; }
    bool WindowEmbedder::unembedWindow(const QString&) { return false; }
}
#endif // _WIN32