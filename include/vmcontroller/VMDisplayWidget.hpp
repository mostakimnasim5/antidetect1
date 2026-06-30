/**
 * VMDisplayWidget.hpp
 * 
 * Qt UI Container Widget for embedding Scrcpy display
 * Handles resize events, mouse/keyboard input, and drag-drop
 */

#pragma once

#include "../VirtualPhonePro.hpp"
#include <QWidget>
#include <QFrame>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMouseEvent>
#include <QKeyEvent>

#ifdef _WIN32
#include <windows.h>
#endif

namespace VirtualPhonePro {

class VMDisplayWidget : public QFrame {
    Q_OBJECT

public:
    explicit VMDisplayWidget(QWidget* parent = nullptr);
    ~VMDisplayWidget();
    
    // Display Management
    void setProfileId(const QString& profileId);
    QString getProfileId() const;
    
    void setEmbeddedWindow(HWND windowHandle);
    void clearEmbeddedWindow();
    HWND getEmbeddedWindow() const;
    
    bool isWindowEmbedded() const;
    
    // Display Settings
    void setDeviceOrientation(bool landscape);
    bool isLandscape() const;
    void toggleOrientation();
    
    void setDeviceModel(const QString& model);
    QString getDeviceModel() const;
    
    void setShowBorder(bool show);
    
    // Screen Scaling
    void setDeviceScreenSize(int width, int height);
    QSize getDeviceScreenSize() const;
    QSize getDisplaySize() const;
    qreal getScaleFactor() const;
    
    // Input Methods
    void sendTap(int x, int y);
    void sendSwipe(int x1, int y1, int x2, int y2, int durationMs = 300);
    void sendText(const QString& text);
    void sendKey(int keyCode);
    void sendHomePress();
    void sendBackPress();
    void sendPowerPress();
    void sendVolumeUp();
    void sendVolumeDown();
    void pullNotificationBar();
    
    // APK Installation
    void installAPK(const QString& apkPath);
    
    // Status
    void setStatus(const QString& status);
    QString getStatus() const;
    void setConnected(bool connected);
    bool isConnected() const;
    
    // Hardware Buttons
    void addHardwareButtons(QWidget* parent);
    
signals:
    void tapRequested(int x, int y);
    void swipeRequested(int x1, int y1, int x2, int y2, int durationMs);
    void textRequested(const QString& text);
    void keyRequested(int keyCode);
    void homePressed();
    void backPressed();
    void powerPressed();
    void volumeUpPressed();
    void volumeDownPressed();
    void notificationBarPulled();
    void orientationChanged(bool landscape);
    void apDropped(const QString& apkPath);
    void embeddedWindowChanged(HWND hwnd);
    
    // Signals for external handling
    void tap(int x, int y);
    void swipe(int x1, int y1, int x2, int y2, int durationMs);
    void textInput(const QString& text);
    void keyEvent(int keyCode);
    void buttonPressed(const QString& button);

protected:
    // Event overrides
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    
    // Focus handling
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    bool event(QEvent* event) override;

private slots:
    void onWindowResized();
    void onDragHoverTimeout();
    void installDroppedAPK();

private:
    // Helper methods
    void updateScaleFactor();
    QPoint mapToDeviceCoordinates(const QPoint& widgetPos);
    QPoint mapFromDeviceCoordinates(const QPoint& devicePos);
    
    void setupUI();
    void createHardwareButtons();
    void createDropOverlay();
    
    bool handleMouseEvent(QMouseEvent* event);
    bool handleKeyboardEvent(QKeyEvent* event);
    
    void sendADBCommand(const QStringList& args);
    
    // Touch simulation
    void simulateTouch(int x, int y, bool isDown);
    void simulateSwipe(int x1, int y1, int x2, int y2, int durationMs);
    void simulateLongPress(int x, int y, int durationMs);
    
    // State
    QString m_profileId;
    HWND m_embeddedWindow;
    bool m_isLandscape;
    QString m_deviceModel;
    bool m_connected;
    QString m_status;
    
    // Scaling
    QSize m_deviceScreenSize;
    qreal m_scaleFactor;
    int m_offsetX;
    int m_offsetY;
    
    // Touch state
    bool m_isTouching;
    QPoint m_touchStartPos;
    QPoint m_touchCurrentPos;
    QTimer* m_swipeTimer;
    
    // Drag-drop
    bool m_isDragHovering;
    QLabel* m_dropOverlay;
    QString m_pendingAPKPath;
    QTimer* m_dropTimer;
    
    // UI Elements
    QVBoxLayout* m_mainLayout;
    QLabel* m_statusLabel;
    QWidget* m_hardwareButtonContainer;
    
    // Control buttons
    QPushButton* m_btnHome;
    QPushButton* m_btnBack;
    QPushButton* m_btnPower;
    QPushButton* m_btnVolumeUp;
    QPushButton* m_btnVolumeDown;
    QPushButton* m_btnRotate;
    QPushButton* m_btnScreenshot;
    QPushButton* m_btnNotification;
    
    // ADB path
    QString m_adbPath;
};

} // namespace VirtualPhonePro