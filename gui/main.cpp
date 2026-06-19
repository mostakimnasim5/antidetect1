#include <QApplication>
#include <QMessageBox>
#include <QSplashScreen>
#include <QPixmap>
#include <QTimer>
#include <QStyleFactory>

#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    app.setApplicationName("AntiDetectPro");
    app.setApplicationVersion("1.0.0");
    app.setApplicationDisplayName("AntiDetectPro - Enterprise Anti-Detection System");
    app.setOrganizationName("AntiDetectPro");
    app.setOrganizationDomain("antidetectpro.com");
    
    app.setStyle(QStyleFactory::create("Fusion"));
    
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    
    app.setPalette(darkPalette);
    
    QPixmap pixmap(400, 200);
    pixmap.fill(Qt::darkBlue);
    QSplashScreen splash(pixmap);
    splash.show();
    splash.setMessage("<font color='white'><b>AntiDetectPro v1.0.0</b><br/>Loading...</font>");
    app.processEvents();
    
    QTimer::singleShot(1000, [&splash]() {
        splash.setMessage("<font color='white'><b>AntiDetectPro v1.0.0</b><br/>Initializing modules...</font>");
    });
    app.processEvents();
    
    MainWindow window;
    window.show();
    
    QTimer::singleShot(500, &splash, &QSplashScreen::close);
    
    return app.exec();
}
