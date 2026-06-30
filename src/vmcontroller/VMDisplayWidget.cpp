/**
 * VMDisplayWidget.cpp
 * 
 * Implementation of Qt UI Container Widget for Scrcpy display
 */

#include "vmcontroller/VMDisplayWidget.hpp"
#include <QPainter>
#include <QApplication>
#include <QScreen>
#include <QFileInfo>
#include <QProcess>
#include <QMessageBox>

#ifdef _WIN32
#include <windows.h>
#endif

namespace VirtualPhonePro {

// ============================================================================
// Constructor & Destructor
// ============================================================================
VMDisplayWidget::VMDisplayWidget(QWidget* parent)
    : QFrame(parent)
    , m_embeddedWindow(nullptr)
    , m_isLandscape(false)
    , m_connected(false)
    , m_scaleFactor(1.0)
    , m_offsetX(0)
    , m_offsetY(0)
    , m_isTouching(false)
    , m_isDragHovering(false)
{
    // Initialize
#ifdef _WIN32
    m_adbPath = "adb.exe";
#else
    m_adbPath = "adb";
#endif
    
    m_deviceScreenSize = QSize(1080, 2400); // Default Android phone size
    
    // Setup UI
    setupUI();
    
    // Enable drag-drop
    setAcceptDrops(true);
    
    // Create timers
    m_swipeTimer = new QTimer(this);
    m_dropTimer = new QTimer(this);
    
    // Set focus policy
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    
    // Style
    setFrameStyle(QFrame::Box | QFrame::Plain);
    setLineWidth(2);
    setStyleSheet("VMDisplayWidget { border: 2px solid #333; background: #1a1a1a; }");
}

VMDisplayWidget::~VMDisplayWidget() {
    clearEmbeddedWindow();
}

// ============================================================================
// Setup
// ============================================================================
void VMDisplayWidget::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // Status label
    m_statusLabel = new QLabel("Disconnected", this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("QLabel { background: rgba(0,0,0,200); color: white; padding: 4px; font-size: 11px; }");
    m_statusLabel->setFixedHeight(20);
    m_statusLabel->hide();
    
    // Drop overlay
    createDropOverlay();
    
    // Hardware buttons container
    createHardwareButtons();
}

void VMDisplayWidget::createDropOverlay() {
    m_dropOverlay = new QLabel(this);
    m_dropOverlay->setAlignment(Qt::AlignCenter);
    m_dropOverlay->setStyleSheet("QLabel { background: rgba(0, 150, 255, 150); border: 3px dashed #0066ff; color: white; font-size: 18px; font-weight: bold; }");
    m_dropOverlay->setText("Drop APK to Install");
    m_dropOverlay->hide();
    m_dropOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);
}

void VMDisplayWidget::createHardwareButtons() {
    m_hardwareButtonContainer = new QWidget(this);
    m_hardwareButtonContainer->setFixedHeight(40);
    m_hardwareButtonContainer->setStyleSheet("QWidget { background: #2a2a2a; border-top: 1px solid #444; }");
    
    QHBoxLayout* btnLayout = new QHBoxLayout(m_hardwareButtonContainer);
    btnLayout->setContentsMargins(8, 4, 8, 4);
    btnLayout->setSpacing(8);
    
    // Create buttons
    m_btnHome = new QPushButton("🏠", m_hardwareButtonContainer);
    m_btnBack = new QPushButton("◀", m_hardwareButtonContainer);
    m_btnPower = new QPushButton("⏻", m_hardwareButtonContainer);
    m_btnVolumeUp = new QPushButton("🔊+", m_hardwareButtonContainer);
    m_btnVolumeDown = new QPushButton("🔊-", m_hardwareButtonContainer);
    m_btnRotate = new QPushButton("↻", m_hardwareButtonContainer);
    m_btnScreenshot = new QPushButton("📷", m_hardwareButtonContainer);
    m_btnNotification = new QPushButton("📬", m_hardwareButtonContainer);
    
    // Style buttons
    QString btnStyle = "QPushButton { min-width: 40px; max-width: 50px; border: none; border-radius: 4px; background: #444; color: white; font-size: 14px; padding: 4px; } QPushButton:hover { background: #555; } QPushButton:pressed { background: #333; }";
    m_btnHome->setStyleSheet(btnStyle);
    m_btnBack->setStyleSheet(btnStyle);
    m_btnPower->setStyleSheet(btnStyle);
    m_btnVolumeUp->setStyleSheet(btnStyle);
    m_btnVolumeDown->setStyleSheet(btnStyle);
    m_btnRotate->setStyleSheet(btnStyle);
    m_btnScreenshot->setStyleSheet(btnStyle);
    m_btnNotification->setStyleSheet(btnStyle);
    
    // Connect signals
    connect(m_btnHome, &QPushButton::clicked, this, [this]() { sendHomePress(); emit homePressed(); emit buttonPressed("home"); });
    connect(m_btnBack, &QPushButton::clicked, this, [this]() { sendBackPress(); emit backPressed(); emit buttonPressed("back"); });
    connect(m_btnPower, &QPushButton::clicked, this, [this]() { sendPowerPress(); emit powerPressed(); emit buttonPressed("power"); });
    connect(m_btnVolumeUp, &QPushButton::clicked, this, [this]() { sendVolumeUp(); emit volumeUpPressed(); emit buttonPressed("volume_up"); });
    connect(m_btnVolumeDown, &QPushButton::clicked, this, [this]() { sendVolumeDown(); emit volumeDownPressed(); emit buttonPressed("volume_down"); });
    connect(m_btnRotate, &QPushButton::clicked, this, [this]() { toggleOrientation(); emit orientationChanged(m_isLandscape); });
    connect(m_btnScreenshot, &QPushButton::clicked, this, &VMDisplayWidget::takeScreenshot);
    connect(m_btnNotification, &QPushButton::clicked, this, [this]() { pullNotificationBar(); emit notificationBarPulled(); });
    
    // Add to layout
    btnLayout->addWidget(m_btnHome);
    btnLayout->addWidget(m_btnBack);
    btnLayout->addWidget(m_btnPower);
    btnLayout->addStretch();
    btnLayout->addWidget(m_btnNotification);
    btnLayout->addWidget(m_btnVolumeUp);
    btnLayout->addWidget(m_btnVolumeDown);
    btnLayout->addWidget(m_btnRotate);
    btnLayout->addWidget(m_btnScreenshot);
}

// ============================================================================
// Display Management
// ============================================================================
void VMDisplayWidget::setProfileId(const QString& profileId) {
    m_profileId = profileId;
}

QString VMDisplayWidget::getProfileId() const {
    return m_profileId;
}

void VMDisplayWidget::setEmbeddedWindow(HWND windowHandle) {
    m_embeddedWindow = windowHandle;
    emit embeddedWindowChanged(windowHandle);
    
    if (windowHandle) {
        setConnected(true);
        setStatus("Connected");
    }
}

void VMDisplayWidget::clearEmbeddedWindow() {
    if (m_embeddedWindow) {
#ifdef _WIN32
        SetParent(m_embeddedWindow, nullptr);
#endif
        m_embeddedWindow = nullptr;
    }
    setConnected(false);
    setStatus("Disconnected");
    emit embeddedWindowChanged(nullptr);
}

HWND VMDisplayWidget::getEmbeddedWindow() const {
    return m_embeddedWindow;
}

bool VMDisplayWidget::isWindowEmbedded() const {
    return m_embeddedWindow != nullptr;
}

// ============================================================================
// Display Settings
// ============================================================================
void VMDisplayWidget::setDeviceOrientation(bool landscape) {
    m_isLandscape = landscape;
    updateScaleFactor();
}

bool VMDisplayWidget::isLandscape() const {
    return m_isLandscape;
}

void VMDisplayWidget::toggleOrientation() {
    m_isLandscape = !m_isLandscape;
    
    // Swap screen dimensions
    int w = m_deviceScreenSize.width();
    int h = m_deviceScreenSize.height();
    m_deviceScreenSize = QSize(h, w);
    
    // Send rotation command to device
    sendADBCommand({ "shell", "content", "insert", "--uri", "content://settings/system", 
                   "--bind", "name:s:user_rotation", "--bind", "value:i:1" });
    
    updateScaleFactor();
}

void VMDisplayWidget::setDeviceModel(const QString& model) {
    m_deviceModel = model;
    if (!m_deviceModel.isEmpty()) {
        setWindowTitle(QString("VirtualPhone - %1").arg(m_deviceModel));
    }
}

QString VMDisplayWidget::getDeviceModel() const {
    return m_deviceModel;
}

void VMDisplayWidget::setShowBorder(bool show) {
    if (show) {
        setFrameStyle(QFrame::Box | QFrame::Plain);
    } else {
        setFrameStyle(QFrame::NoFrame);
    }
}

void VMDisplayWidget::takeScreenshot() {
    // Screenshot implementation
    QString path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QString filename = QString("%1/VirtualPhone_%2.png")
        .arg(path)
        .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));
    
    sendADBCommand({ "exec-out", "screencap", "-p", ">", filename });
}

void VMDisplayWidget::setStatus(const QString& status) {
    m_status = status;
    if (m_statusLabel) {
        m_statusLabel->setText(status);
        m_statusLabel->setVisible(!status.isEmpty());
    }
}

QString VMDisplayWidget::getStatus() const {
    return m_status;
}

void VMDisplayWidget::setConnected(bool connected) {
    m_connected = connected;
    m_hardwareButtonContainer->setEnabled(connected);
}

bool VMDisplayWidget::isConnected() const {
    return m_connected;
}

// ============================================================================
// Screen Scaling
// ============================================================================
void VMDisplayWidget::setDeviceScreenSize(int width, int height) {
    m_deviceScreenSize = QSize(width, height);
    updateScaleFactor();
}

QSize VMDisplayWidget::getDeviceScreenSize() const {
    return m_deviceScreenSize;
}

QSize VMDisplayWidget::getDisplaySize() const {
    // Return the available size for the embedded window
    return QSize(width(), height() - m_hardwareButtonContainer->height());
}

qreal VMDisplayWidget::getScaleFactor() const {
    return m_scaleFactor;
}

void VMDisplayWidget::updateScaleFactor() {
    if (m_deviceScreenSize.isEmpty()) {
        m_scaleFactor = 1.0;
        return;
    }
    
    QSize displaySize = getDisplaySize();
    
    // Calculate scale to fit device screen in display
    qreal scaleX = (qreal)displaySize.width() / m_deviceScreenSize.width();
    qreal scaleY = (qreal)displaySize.height() / m_deviceScreenSize.height();
    
    // Use the smaller scale to ensure full screen fits
    m_scaleFactor = qMin(scaleX, scaleY);
    
    // Center offset
    int scaledWidth = m_deviceScreenSize.width() * m_scaleFactor;
    int scaledHeight = m_deviceScreenSize.height() * m_scaleFactor;
    m_offsetX = (displaySize.width() - scaledWidth) / 2;
    m_offsetY = (displaySize.height() - scaledHeight) / 2;
}

// ============================================================================
// Mouse Events
// ============================================================================
void VMDisplayWidget::mousePressEvent(QMouseEvent* event) {
    if (!m_connected || !m_embeddedWindow) {
        QFrame::mousePressEvent(event);
        return;
    }
    
    if (event->button() == Qt::LeftButton) {
        m_isTouching = true;
        m_touchStartPos = event->pos();
        m_touchCurrentPos = event->pos();
        
        // Get device coordinates
        QPoint devicePos = mapToDeviceCoordinates(event->pos());
        
        // Send touch down
        simulateTouch(devicePos.x(), devicePos.y(), true);
        
        // Start swipe timer for long press detection
        m_swipeTimer->start(500);
    }
}

void VMDisplayWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (!m_connected || !m_embeddedWindow) {
        QFrame::mouseReleaseEvent(event);
        return;
    }
    
    m_swipeTimer->stop();
    
    if (event->button() == Qt::LeftButton) {
        QPoint devicePos = mapToDeviceCoordinates(event->pos());
        
        // Check if this was a swipe
        QLineF swipeLine(m_touchStartPos, event->pos());
        if (swipeLine.length() > 20) {
            // It was a swipe, send swipe command
            QPoint deviceStart = mapToDeviceCoordinates(m_touchStartPos);
            QPoint deviceEnd = mapToDeviceCoordinates(event->pos());
            sendSwipe(deviceStart.x(), deviceStart.y(), deviceEnd.x(), deviceEnd.y(), 200);
        } else {
            // It was a tap
            sendTap(devicePos.x(), devicePos.y());
        }
        
        m_isTouching = false;
    }
}

void VMDisplayWidget::mouseMoveEvent(QMouseEvent* event) {
    if (!m_connected || !m_embeddedWindow) {
        QFrame::mouseMoveEvent(event);
        return;
    }
    
    if (m_isTouching) {
        m_touchCurrentPos = event->pos();
    }
}

void VMDisplayWidget::wheelEvent(QWheelEvent* event) {
    if (!m_connected || !m_embeddedWindow) {
        QFrame::wheelEvent(event);
        return;
    }
    
    // Send scroll event
    QPoint devicePos = mapToDeviceCoordinates(event->pos());
    int scrollAmount = event->angleDelta().y() > 0 ? 100 : -100;
    sendADBCommand({ "shell", "input", "scrollclick", 
                   QString::number(devicePos.x()), 
                   QString::number(devicePos.y()),
                   QString::number(scrollAmount) });
}

QPoint VMDisplayWidget::mapToDeviceCoordinates(const QPoint& widgetPos) {
    // Adjust for offset and scale
    int x = (int)((widgetPos.x() - m_offsetX) / m_scaleFactor);
    int y = (int)((widgetPos.y() - m_offsetY) / m_scaleFactor);
    
    // Clamp to device screen bounds
    x = qBound(0, x, m_deviceScreenSize.width() - 1);
    y = qBound(0, y, m_deviceScreenSize.height() - 1);
    
    return QPoint(x, y);
}

QPoint VMDisplayWidget::mapFromDeviceCoordinates(const QPoint& devicePos) {
    int x = (int)(devicePos.x() * m_scaleFactor + m_offsetX);
    int y = (int)(devicePos.y() * m_scaleFactor + m_offsetY);
    return QPoint(x, y);
}

// ============================================================================
// Keyboard Events
// ============================================================================
void VMDisplayWidget::keyPressEvent(QKeyEvent* event) {
    if (!m_connected || !m_embeddedWindow) {
        QFrame::keyPressEvent(event);
        return;
    }
    
    // Forward text input
    if (!event->text().isEmpty()) {
        sendText(event->text());
        return;
    }
    
    // Handle special keys
    switch (event->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            sendADBCommand({ "shell", "input", "keyevent", "66" }); // KEYCODE_ENTER
            break;
        case Qt::Key_Backspace:
            sendADBCommand({ "shell", "input", "keyevent", "67" }); // KEYCODE_DEL
            break;
        case Qt::Key_Escape:
            sendBackPress();
            break;
        case Qt::Key_Home:
            sendHomePress();
            break;
        case Qt::Key_F1:
            sendVolumeUp();
            break;
        case Qt::Key_F2:
            sendVolumeDown();
            break;
        case Qt::Key_F5:
            toggleOrientation();
            break;
        default:
            QFrame::keyPressEvent(event);
    }
}

void VMDisplayWidget::keyReleaseEvent(QKeyEvent* event) {
    QFrame::keyReleaseEvent(event);
}

// ============================================================================
// Focus Events
// ============================================================================
void VMDisplayWidget::focusInEvent(QFocusEvent* event) {
    setStyleSheet("VMDisplayWidget { border: 3px solid #0078d4; background: #1a1a1a; }");
    QFrame::focusInEvent(event);
}

void VMDisplayWidget::focusOutEvent(QFocusEvent* event) {
    setStyleSheet("VMDisplayWidget { border: 2px solid #333; background: #1a1a1a; }");
    QFrame::focusOutEvent(event);
}

bool VMDisplayWidget::event(QEvent* event) {
    // Handle focus changes for keyboard input
    if (event->type() == QEvent::FocusAboutToChange) {
        if (isActiveWindow() && !hasFocus()) {
            setFocus(Qt::OtherFocusReason);
        }
    }
    return QFrame::event(event);
}

// ============================================================================
// Drag & Drop Events
// ============================================================================
void VMDisplayWidget::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        if (!urls.isEmpty()) {
            QString filePath = urls.first().toLocalFile();
            if (filePath.endsWith(".apk", Qt::CaseInsensitive)) {
                event->acceptProposedAction();
                m_isDragHovering = true;
                m_dropOverlay->setGeometry(rect());
                m_dropOverlay->show();
                return;
            }
        }
    }
    QFrame::dragEnterEvent(event);
}

void VMDisplayWidget::dragMoveEvent(QDragMoveEvent* event) {
    if (m_isDragHovering) {
        event->acceptProposedAction();
        return;
    }
    QFrame::dragMoveEvent(event);
}

void VMDisplayWidget::dragLeaveEvent(QDragLeaveEvent* event) {
    m_isDragHovering = false;
    m_dropOverlay->hide();
    QFrame::dragLeaveEvent(event);
}

void VMDisplayWidget::dropEvent(QDropEvent* event) {
    m_isDragHovering = false;
    m_dropOverlay->hide();
    
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        if (!urls.isEmpty()) {
            QString filePath = urls.first().toLocalFile();
            if (filePath.endsWith(".apk", Qt::CaseInsensitive)) {
                installAPK(filePath);
                emit apDropped(filePath);
                return;
            }
        }
    }
    QFrame::dropEvent(event);
}

void VMDisplayWidget::installAPK(const QString& apkPath) {
    if (!m_connected) {
        QMessageBox::warning(this, "Not Connected", "Device not connected. Cannot install APK.");
        return;
    }
    
    QFileInfo fileInfo(apkPath);
    if (!fileInfo.exists()) {
        QMessageBox::warning(this, "File Not Found", "APK file not found: " + apkPath);
        return;
    }
    
    setStatus("Installing APK...");
    
    // Install APK via ADB
    QStringList args = { "install", "-r", apkPath };
    sendADBCommand(args);
    
    setStatus("APK Installed");
    QMessageBox::information(this, "Success", "APK installed successfully: " + fileInfo.fileName());
}

void VMDisplayWidget::installDroppedAPK() {
    if (!m_pendingAPKPath.isEmpty()) {
        installAPK(m_pendingAPKPath);
        m_pendingAPKPath.clear();
    }
}

void VMDisplayWidget::onDragHoverTimeout() {
    m_dropOverlay->hide();
    m_isDragHovering = false;
}

// ============================================================================
// Input Methods
// ============================================================================
void VMDisplayWidget::sendTap(int x, int y) {
    sendADBCommand({ "shell", "input", "tap", 
                   QString::number(x), QString::number(y) });
    emit tap(x, y);
    emit tapRequested(x, y);
}

void VMDisplayWidget::sendSwipe(int x1, int y1, int x2, int y2, int durationMs) {
    sendADBCommand({ "shell", "input", "swipe",
                   QString::number(x1), QString::number(y1),
                   QString::number(x2), QString::number(y2),
                   QString::number(durationMs) });
    emit swipe(x1, y1, x2, y2, durationMs);
    emit swipeRequested(x1, y1, x2, y2, durationMs);
}

void VMDisplayWidget::sendText(const QString& text) {
    // Escape special characters
    QString escaped = text;
    escaped.replace(" ", "%s");
    escaped.replace("'", "\\'");
    escaped.replace("\"", "\\\"");
    
    sendADBCommand({ "shell", "input", "text", escaped });
    emit textInput(text);
    emit textRequested(text);
}

void VMDisplayWidget::sendKey(int keyCode) {
    sendADBCommand({ "shell", "input", "keyevent", QString::number(keyCode) });
    emit keyEvent(keyCode);
    emit keyRequested(keyCode);
}

void VMDisplayWidget::sendHomePress() {
    sendKey(3); // KEYCODE_HOME
}

void VMDisplayWidget::sendBackPress() {
    sendKey(4); // KEYCODE_BACK
}

void VMDisplayWidget::sendPowerPress() {
    sendKey(26); // KEYCODE_POWER
}

void VMDisplayWidget::sendVolumeUp() {
    sendKey(24); // KEYCODE_VOLUME_UP
}

void VMDisplayWidget::sendVolumeDown() {
    sendKey(25); // KEYCODE_VOLUME_DOWN
}

void VMDisplayWidget::pullNotificationBar() {
    // Swipe from top
    sendSwipe(m_deviceScreenSize.width() / 2, 100, 
              m_deviceScreenSize.width() / 2, 
              m_deviceScreenSize.height() / 2, 300);
}

void VMDisplayWidget::sendADBCommand(const QStringList& args) {
    if (!m_connected) return;
    
    QStringList adbArgs = { "-s", QString("127.0.0.1:%1").arg(getProfileId().isEmpty() ? "5555" : getProfileId()) };
    adbArgs.append(args);
    
    QProcess* proc = new QProcess(this);
    proc->start(m_adbPath, adbArgs);
    proc->waitForFinished(5000);
    proc->deleteLater();
}

// ============================================================================
// Touch Simulation
// ============================================================================
void VMDisplayWidget::simulateTouch(int x, int y, bool isDown) {
    // Touch is handled via ADB
    if (isDown) {
        // Store touch start position
        m_touchStartPos = mapFromDeviceCoordinates(QPoint(x, y));
    }
}

void VMDisplayWidget::simulateSwipe(int x1, int y1, int x2, int y2, int durationMs) {
    sendSwipe(x1, y1, x2, y2, durationMs);
}

void VMDisplayWidget::simulateLongPress(int x, int y, int durationMs) {
    sendADBCommand({ "shell", "input", "swipe",
                   QString::number(x), QString::number(y),
                   QString::number(x), QString::number(y),
                   QString::number(durationMs) });
}

// ============================================================================
// Resize Events
// ============================================================================
void VMDisplayWidget::resizeEvent(QResizeEvent* event) {
    QFrame::resizeEvent(event);
    updateScaleFactor();
    onWindowResized();
}

void VMDisplayWidget::onWindowResized() {
    if (m_embeddedWindow) {
#ifdef _WIN32
        SetWindowPos(m_embeddedWindow, HWND_TOP, 
                    0, 0, 
                    width(), 
                    height() - m_hardwareButtonContainer->height(),
                    SWP_NOZORDER | SWP_SHOWWINDOW);
#endif
    }
    
    // Update drop overlay
    if (m_dropOverlay) {
        m_dropOverlay->setGeometry(rect());
    }
}

} // namespace VirtualPhonePro