#ifndef NATIVE_WINDOW_WIDGET
#define NATIVE_WINDOW_WIDGET

#include <QWidget>
#include <memory>


// Qt widget that represents a completely blank native window, for use as a 
// rendering target
class NativeWindowWidget : public QWidget
{
  // Qt moc magic
  Q_OBJECT

public:
  // Constructor marked explicit to prevent other widgets from implicitly
  // casting to a native window
  explicit NativeWindowWidget(QWidget* i_parent) noexcept;
  // Explicitly deleted default constructor
  NativeWindowWidget() noexcept = delete;
  // Explicitly defaulted virtual destructor
  virtual ~NativeWindowWidget() noexcept = default;
  // Return a null pointer here to tell Qt we're handling all drawing
  virtual QPaintEngine* paintEngine() const override final;
  // Initialization function, to be called after the widget has already been
  // set-up, from here we can access the native window ID
  virtual void init();

protected:
  // For derived classes to draw to this native window
  virtual void draw_impl();
  // For derived classes to handle a window resize
  virtual void resize_impl(void* io_native_window);
  // For derived classes to run initialization procedures
  virtual void init_impl(void* io_native_window);

private:
  // Call this function to request a redraw of the window
  void request_draw();
  // This event will simply request a draw
  virtual void paintEvent(QPaintEvent* i_paint_event) override final;
  // This event will simply delegate to resize_impl after boilerplate check,
  // and then request a draw
  virtual void resizeEvent(QResizeEvent* i_resize_event) override final;
  // We need to intercept the update request, and call draw_impl when received
  virtual bool event(QEvent* i_event) override final;

protected:
  // Has this window been initialized?
  bool m_is_init;

private:
  // Has a draw been requested?
  bool m_update_pending;

};

#endif  // NATIVE_WINDOW_WIDGET
