#include "utility.h"
//#include <string.h>
#include <libgen.h>
#include <pwd.h>
#include <unistd.h>
#include <vector>
#include <QGraphicsView>
#include <QImage>
#include <QSettings>

namespace TivUtility {

std::string Basename(const std::string& path) {
  std::vector<char> vPath(path.size()+1);
  strcpy(&vPath[0], path.c_str());
  char* rv = basename(&vPath[0]);
  return rv;
}
std::string Dirname(const std::string& path) {
  std::vector<char> vPath(path.size()+1);
  strcpy(&vPath[0], path.c_str());
  char* rv = dirname(&vPath[0]);
  return rv;
}
std::string HomeDir() {
  const char* dir;
  std::string rv;
  if ((dir = getenv("HOME")) != nullptr) {
    rv = dir;
  } else {
    rv = getpwuid(getuid())->pw_dir;
  }
  return rv + "/";
}

void saveSetting(const char* key, const char* value) {
  std::string settingsFile = HomeDir() + "/.tivrc";
  QSettings settings(QString(settingsFile.c_str()), QSettings::NativeFormat);

  settings.setValue(key, value);
}
std::string loadSetting(const char* key, const char* defaultValue) {
  std::string settingsFile = HomeDir() + "/.tivrc";
  QSettings settings(QString(settingsFile.c_str()), QSettings::NativeFormat);

  QString qValue = settings.value(key, defaultValue).toString();
  return qValue.toUtf8().constData();
}

// Maximize the MDI subwindow and display its image at max size
void maximize(QGraphicsView* v, const QImage& image) {
  v->resetTransform();    // Reset any previous scale()
  v->showMaximized();     // Maximize the MDI subwindow
  double targetScale = getScale<const QSize&>(image, v->size());
  v->scale(targetScale, targetScale); // Scale the image to fit the maximized MDI subwindow
}

}
