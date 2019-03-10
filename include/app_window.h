#ifndef APP_WINDOW
#define APP_WINDOW

#include <memory>
#include <QMainWindow>
#include "ui_applayout.h"

// Forward declare our native window widget, could include it instead
QT_FORWARD_DECLARE_CLASS(NativeWindowWidget)


// Basic application window that uses a Qt ui form
class AppWindow final : public QMainWindow
{
  // Qt moc magic
  Q_OBJECT

public:
  // Explicit constructor for the main window. It is explicit so QWidgets can't
  // be implicitly cast to AppWindows
  explicit AppWindow(QWidget *io_parent = nullptr) noexcept;
  // Default destructor
  ~AppWindow() override = default;
  // Used to initialize the application window with a native window widget, 
  // placing it as the central widget.
  void init(std::shared_ptr<NativeWindowWidget> i_window);

private:
  // Used to handle a key press, will get delegated to the scene.
  void keyPressEvent(QKeyEvent * io_event) override;
  // The Qt UI form.
  Ui::AppLayout m_ui_layout;
  // A pointer to the native window that should be placed in the central widget.
  std::shared_ptr<NativeWindowWidget> m_native_window = nullptr;

};


#endif // APP_WINDOW
