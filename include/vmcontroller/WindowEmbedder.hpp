/**
 * WindowEmbedder.hpp
 * 
 * Win32 API Integration for embedding Scrcpy windows into Qt UI
 * Handles window search, embedding, styling, and event handling
 */

#pragma once

#include "../VirtualPhonePro.hpp"
#include <QObject>
#include <QString>
#include <QWindow>
#include <QTimer>
#include <QMutex>
#include <WINDOWS.H>

namespace VirtualPhonePro {

class WindowEmbedder : public QObject {
    Q_OBJECT

public:
    static WindowEmbedder& getInstance();
    
    // Lifecycle
    bool initialize();
    void shutdown();
    
    // Window Embedding
    bool embedWindow(const QString& windowTitle, WId containerId);
    bool embedWindowByHandle(HWND windowHandle, WId containerId);
    bool unembedWindow(const QString& windowTitle);
    bool unembedAllWindows();
    
    // Window Search
    HWND findWindowByTitle(const QString& title);
    HWND findWindowByClass(const QString& className);
    QList<HWND> findAllWindows(const QString& partialTitle);
    
    // Window Control
    bool resizeWindow(HWND hwnd, int width, int height);
    bool moveWindow(HWND hwnd, int x, int y);
    bool setWindowSizeToContainer(HWND hwnd, WId containerId);
    bool bringToFront(HWND hwnd);
    bool hideOriginalChrome(HWND hwnd);
    bool showOriginalChrome(HWND hwnd);
    
    // Styling
    bool applyChildStyle(HWND hwnd);
    bool removeTitleBar(HWND hwnd);
    bool removeBorder(HWND hwnd);
    bool removeAllChrome(HWND hwnd);
    bool setWindowTransparent(HWND hwnd, bool transparent);
    
    // Event Handling
    bool redirectInput(HWND sourceHwnd, HWND targetHwnd);
    bool forwardMouseEvents(HWND source, HWND target);
    bool forwardKeyboardEvents(HWND source, HWND target);
    
    // Monitoring
    bool startWindowMonitoring(const QString& windowTitle, int intervalMs = 500);
    bool stopWindowMonitoring(const QString& windowTitle);
    void stopAllMonitoring();
    
    // State Query
    bool isWindowEmbedded(HWND hwnd) const;
    bool isWindowEmbeddedByTitle(const QString& title) const;
    HWND getEmbeddedWindowHandle(const QString& title) const;
    QString getWindowTitle(HWND hwnd) const;
    QStringList getEmbeddedWindowTitles() const;
    
    // Container Management
    bool attachContainer(const QString& windowTitle, WId containerId);
    bool detachContainer(const QString& windowTitle);
    bool updateContainerSize(const QString& windowTitle);
    
signals:
    void windowFound(const QString& title, HWND handle);
    void windowLost(const QString& title, HWND handle);
    void windowEmbedded(const QString& title, HWND handle);
    void windowUnembedded(const QString& title);
    void windowResized(const QString& title, int width, int height);
    void embeddingError(const QString& title, const QString& error);

private:
    explicit WindowEmbedder(QObject* parent = nullptr);
    ~WindowEmbedder();
    WindowEmbedder(const WindowEmbedder&) = delete;
    WindowEmbedder& operator=(const WindowEmbedder&) = delete;
    
    // Win32 Callbacks
    static BOOL CALLBACK enumWindowsCallback(HWND hwnd, LPARAM lParam);
    static BOOL CALLBACK enumChildWindowsCallback(HWND hwnd, LPARAM lParam);
    
    // Internal helpers
    bool setWindowStyle(HWND hwnd, DWORD addStyle, DWORD removeStyle);
    bool updateEmbeddedWindowPosition(const QString& title);
    void cleanupEmbeddedWindow(const QString& title);
    
    // Container management
    struct EmbeddedWindowInfo {
        HWND hwnd;
        WId containerId;
        QString title;
        bool isEmbedded;
        QTimer* resizeTimer;
    };
    
    QMap<QString, EmbeddedWindowInfo> m_embeddedWindows;
    QMap<HWND, QString> m_handleToTitle;
    QMutex m_mutex;
    
    // Monitoring timers
    QMap<QString, QTimer*> m_monitoringTimers;
    
    // Original window styles (for restoration)
    QMap<HWND, LONG> m_originalStyles;
    QMap<HWND, LONG> m_originalExStyles;
};

} // namespace VirtualPhonePro