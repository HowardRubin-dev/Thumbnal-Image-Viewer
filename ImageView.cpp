#include "ImageView.h"
#include "utility.h"
#include "printimage.h"
#include "trashfiles.h"
#include <QPrinter>
#include <QGraphicsPixmapItem>
//#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QFileDialog>

ImageView::ImageView(QGraphicsScene* scene, QImage& im, const std::string& fn)
  : QGraphicsView(scene), qImage(im), sFilename(fn)
{
}

ImageView* ImageView::open() {
  QString filename =  QFileDialog::getOpenFileName(nullptr, "Open Image",
						   (TivUtility::HomeDir() + "Pictures/").c_str(),
						   "Images (*.jpg  *.jpeg  *.png  *.gif) ;; All files (*.*)");
  
  return filename.isEmpty() ? nullptr : openFile(filename);
}

ImageView* ImageView::openFile(const QString& filename) {
  QImage image;
  QGraphicsPixmapItem* item;
  ImageView* view = nullptr;

  if (image.load(filename)
      && (item = new QGraphicsPixmapItem(QPixmap::fromImage(image))) != nullptr) {

    QGraphicsScene* scene = new QGraphicsScene();
    scene->addItem(item);

    view = new ImageView(scene, image, filename.toStdString());
  }

  return view;
}

void ImageView::zoomIn() {
  scale(1.25, 1.25);
}
void ImageView::zoomOut() {
  scale(1/1.25, 1/1.25);
}
void ImageView::normalSize() {
  resetTransform();
}
// Maximize the MDI subwindow and display its image at max size
void ImageView::maximize() {
  TivUtility::maximize(this, qImage);
}

void ImageView::Print() {
  PrintImage pi(qImage);
  pi.Print();
}
void ImageView::trashFile() {
  /*TrashFiles::TrashResult result = */
  TrashFiles().TrashOneFile(sFilename.c_str());
  //enum class TrashResult { SUCCESS, ERROR, MISSING_FILE, };
}
