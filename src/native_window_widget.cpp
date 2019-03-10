#include "native_window_widget.h"
#include <mutex>
#include <QApplication>
#include <QResizeEvent>

NativeWindowWidget::NativeWindowWidget(QWidget* i_parent) noexcept
  : QWidget(i_parent), m_is_init(false), m_update_pending(false)
{
  setAttribute(Qt::WA_NativeWindow);
  setAttribute(Qt::WA_PaintOnScreen);
  setAttribute(Qt::WA_NoSystemBackground);
}

QPaintEngine* NativeWindowWidget::paintEngine() const
{
  return nullptr;
}

void NativeWindowWidget::request_draw()
{
  if (m_update_pending == false)
  {
    m_update_pending = true;
    QApplication::postEvent(this, new QEvent{QEvent::UpdateRequest});
  }
}

void NativeWindowWidget::paintEvent(QPaintEvent* /*i_paint_event*/)
{
  // Register a request to draw our window
  request_draw();
}

void NativeWindowWidget::resizeEvent(QResizeEvent* i_resize_event)
{
  QWidget::resizeEvent(i_resize_event);

  // Get our previous and current window sizes
  const auto old_size = i_resize_event->oldSize();
  const auto size = i_resize_event->size();

  // Don't resize to invalid negative dimensions
  if (size.width() < 0 || size.height() < 0)
    return;

  resize_impl();

  // Qt automatically requests an update if we resize the window to larger
  // dimensions, but if we resize smaller, we need to manually request the
  // update
  if (size.width() < old_size.width() || size.height() < old_size.height())
  {
    request_draw();
  }
}

bool NativeWindowWidget::event(QEvent* i_event)
{
  switch (i_event->type())
  {
  // If an update has been requested, we need to draw
  case QEvent::UpdateRequest:
  {
    // Only draw if the window is visible
    if (isVisible())
      draw_impl();
    // Set this to false now that we've satisfied the request
    m_update_pending = false;
    return true;
  }
  // All other events should have default behavior
  default: return QWidget::event(i_event);
  }
}

void NativeWindowWidget::init()
{
  // Quick exit to avoid locking
  if (m_is_init)
    return;
  // Initialize using the native window id, exactly once in a thread safe manor
  static std::once_flag is_init;
  std::call_once(is_init, [this] {
    m_is_init = true;
    auto native_window_id = winId();
    init_impl((void*)native_window_id);
  });
}

//------------------------------------------------------------------------------
//----------------Default implementations do nothing----------------------------
//------------------------------------------------------------------------------
void NativeWindowWidget::draw_impl()
{
}
void NativeWindowWidget::resize_impl()
{
}
void NativeWindowWidget::init_impl(void*)
{
}

