#pragma once
#include <QGraphicsView>

class SlideshowView : public QGraphicsView {
  Q_OBJECT

 public:
  SlideshowView(const std::string& fn);
  ~SlideshowView();

  enum class SlideshowAction { PREVIOUS, NEXT, FIRST, LAST,
      ZOOMIN, ZOOMOUT, NORMALSIZE, MAXIMIZE,
      PRINT, /*CANCEL,*/ DELETE_FILE, TRASH_FILE, };
  void UserAction(SlideshowAction result);

  void newSlide(int r, const std::string& fn);
  int getRow() const { return row; }
  const std::string& filename() const { return sFilename; }

  void setId(void* id) { listviewSubwindowId = id; }

signals:
  void signalkeyPress(void* id, SlideshowAction action);

 private:
  SlideshowAction currentZoom = SlideshowAction::NORMALSIZE;
  QImage image;
  std::string sFilename;
  int row = 0;
  void* listviewSubwindowId = nullptr;
};
