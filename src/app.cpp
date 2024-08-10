//
// Created by benichn on 29/07/24.
//

#include "app.h"

bool GlobalEventFilter::eventFilter(QObject* obj, QEvent* e)
{
    if (const auto type = e->type(); type == QEvent::KeyPress || type == QEvent::KeyRelease)
    {
        switch (const auto keyEvent = dynamic_cast<QKeyEvent*>(e); keyEvent->key())
        {
        case Qt::Key_Control:
        case Qt::Key_Shift:
        case Qt::Key_Alt:
        case Qt::Key_AltGr:
        case Qt::Key_Meta:
        case Qt::Key_Super_L:
        case Qt::Key_Super_R:
            emit modifiersChangeEvent(QGuiApplication::queryKeyboardModifiers());
            break;
        default: ;
        }
    }
    return QObject::eventFilter(obj, e);
}

App::App(int& argc, char** argv): QApplication(argc, argv), _book({
                                      "/home/benichn/prog/cpp/scan/testBook",
                                      "Test",
                                      {
                                          {
                                              "Recadrage",
                                              {
                                                  {"flip", true},
                                                  {"maxBlockDist", 60},
                                                  {"minConnectedBlockSize", 250},
                                                  {"minBlockSize", 10},
                                                  {"cropOverflow", 50},
                                                  {"whiteThreshold", 234},
                                                  {"smallImageBlocksArea", 20},
                                                  {"whiteColorThreshold", 240},
                                                  {"colorGamma", 0.5},
                                                  {"maxBigCCColorMean", 15000},
                                                  {"blurSize", QSize(110, 12)},
                                                  {"maxBlurredCCArea", 4000}
                                              }
                                          },
                                          {
                                              "Finalisation", {
                                                  {"dpi", 600},
                                                  {"finalWidthCm", 15.0f},
                                                  {"finalHeightCm", 23.3f},
                                                  {"leftMarginCm", 1.7f},
                                                  {"rightMarginCm", 1.7f},
                                                  {"topMarginCm", 1.4f},
                                                  {"bottomMarginCm", 1.4f},
                                                  {"forcedLeftCm", -1.0f},
                                                  {"forcedRightCm", -1.0f},
                                                  {"forcedTopCm", -1.0f},
                                                  {"forcedBottomCm", -1.0f},
                                                  {"headerHeightCm", 2.75f},
                                                  {"alignmentH", "l"},
                                                  {"alignmentV", "t"},
                                              }
                                          },
                                      }
                                  })
{
    installEventFilter(&_eventFilter);
    QImageReader::setAllocationLimit(512);
}
