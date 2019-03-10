#include "app_window.h"
#include "native_window_widget.h"
#include <QKeyEvent>

AppWindow::AppWindow(QWidget* io_parent) noexcept : QMainWindow(io_parent)
{
}

void AppWindow::init(std::shared_ptr<NativeWindowWidget> i_window)
{
  m_native_window = std::move(i_window);
  m_ui_layout.setupUi(this);
  m_ui_layout.grid_layout->addWidget(m_native_window.get(), 0, 0, 3, 5);
}

void AppWindow::keyPressEvent(QKeyEvent* io_event)
{
  switch (io_event->key())
  {
    // Quit when escape key is hit
  case Qt::Key_Escape: QApplication::exit(EXIT_SUCCESS); break;
  default: break;
  }
}

