#include <QGraphicsPixmapItem>
#include <QKeyEvent>
#include "SlideshowView.h"
#include "utility.h"
#include "printimage.h"

SlideshowView::SlideshowView(const std::string& fn) : sFilename(fn) {
  QGraphicsPixmapItem* item;

  if (image.load(fn.c_str())
      && (item = new QGraphicsPixmapItem(QPixmap::fromImage(image))) != nullptr) {

    QGraphicsScene* scene = new QGraphicsScene();
    scene->addItem(item);

    setScene(scene);
  }
}

SlideshowView::~SlideshowView() {
  delete scene();
}

void SlideshowView::newSlide(int r, const std::string& fn) {
  QGraphicsPixmapItem* item;

  if (image.load(fn.c_str())
      && (item = new QGraphicsPixmapItem(QPixmap::fromImage(image))) != nullptr) {

    row = r;
    sFilename = fn;

    QGraphicsScene* newScene = new QGraphicsScene();
    newScene->addItem(item);

    QGraphicsScene* oldScene = scene();
    setScene(newScene);
    delete oldScene;
  }
}

void SlideshowView::UserAction(SlideshowAction action) {
  switch (action) {
  case SlideshowAction::ZOOMIN:     scale(1.25, 1.25);                 break;
  case SlideshowAction::ZOOMOUT:    scale(1/1.25, 1/1.25);             break;
  case SlideshowAction::NORMALSIZE: resetTransform();                  break;
  case SlideshowAction::MAXIMIZE:   TivUtility::maximize(this, image); break;
  case SlideshowAction::PRINT: {
    PrintImage p(image);
    p.Print();
  }
    break;
  default: emit signalkeyPress(listviewSubwindowId, action); break;
  }

  if (currentZoom == SlideshowAction::MAXIMIZE
      && TivUtility::among(action, { SlideshowAction::PREVIOUS,
				     SlideshowAction::NEXT,
				     SlideshowAction::FIRST,
				     SlideshowAction::LAST })) {
    TivUtility::maximize(this, image);
  }
  if (TivUtility::among(action, { SlideshowAction::ZOOMIN,
				  SlideshowAction::ZOOMOUT,
				  SlideshowAction::NORMALSIZE,
				  SlideshowAction::MAXIMIZE })) {
    currentZoom = action;
  }
}
