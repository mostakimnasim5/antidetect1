/**
 * VirtualPhonePro GUI - Qt6 Main Entry Point
 */

#include <QApplication>
#include "MainWindow.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    app.setApplicationName("VirtualPhonePro");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("VirtualPhonePro");
    
    MainWindow window;
    window.show();
    
    return app.exec();
}
