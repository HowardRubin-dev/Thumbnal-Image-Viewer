#pragma once
#include <QGraphicsView>

class QPrinter;

class ImageView : public QGraphicsView {
  Q_OBJECT

 public:
  ImageView(QGraphicsScene* scene, QImage& im, const std::string& fn);
  static ImageView* open();
  static ImageView* openFile(const QString& filename);

  const std::string& filename() { return sFilename; }

  void zoomIn();
  void zoomOut();
  void normalSize();
  void maximize();
  void trashFile();

  void Print();

 private:
  const QImage qImage;
  const std::string sFilename;
};
