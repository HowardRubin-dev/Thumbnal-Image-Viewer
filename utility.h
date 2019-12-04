#pragma once
#include <QImage>
#include <QCursor>
#include <string>
class QGraphicsView;

namespace TivUtility {
std::string Basename(const std::string& path);
std::string Dirname(const std::string& path);
std::string HomeDir();

void saveSetting(const char* key, const char* value);
std::string loadSetting(const char* key, const char* defaultValue="");

// Maximize the MDI subwindow and display its image at max size
void maximize(QGraphicsView* v, const QImage& image);

template <typename T>
class WaitCursor {
public:
  WaitCursor(T* t) : m_t(t) {
    m_t->setCursor(Qt::WaitCursor);
  }
  ~WaitCursor() {
    m_t->setCursor(Qt::ArrowCursor);
  }
private:
  T* const m_t = nullptr;
};

template <typename T>
double getScale(const QImage& qImage, const T& out) {
  double imgWidth  = qImage.width(),
	 imgHeight = qImage.height(),
	 xScale    = out.width()  / imgWidth,
	 yScale    = out.height() / imgHeight,
	 targetScale = qMin(xScale, yScale);
  return targetScale;
}

// Determine whether target is among values
template <typename T>
bool among(const T& target, const std::initializer_list<const T>& values) {
  return std::find(values.begin(), values.end(), target) != values.end();
}
}
